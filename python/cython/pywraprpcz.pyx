from cpython cimport Py_DECREF, Py_INCREF
from cython.operator cimport dereference as deref
from libc.stdlib cimport malloc, free


cdef extern from "Python.h":
    void PyEval_InitThreads()


def init():
    import sys
    PyEval_InitThreads()


init()


cdef extern from "string" namespace "std":
    cdef cppclass string:
        string()
        string(char*)
        string(char*, size_t)
        size_t size()
        char* c_str()


cdef string make_string(pystring) except *:
    return string(pystring, len(pystring))


cdef string_ptr_to_pystring(string* s):
    return s.c_str()[:s.size()]


cdef cstring_to_pystring(void* s, size_t size):
    return (<char*>s)[:size]


cdef string_to_pystring(string s):
    return s.c_str()[:s.size()]


cdef extern from "rpcz/sync_event.hpp" namespace "rpcz":
    cdef cppclass _sync_event "rpcz::sync_event":
        void signal() nogil
        void wait() nogil

cdef extern from "rpcz/rpc_controller.hpp" namespace "rpcz":
    cdef cppclass _rpc_controller "rpcz::rpc_controller":
        bint ok()
        int get_status()
        string get_error_message()
        int get_application_error_code()
        long get_deadline_ms()
        void set_deadline_ms(long)
        int wait() nogil


cdef class RpcController:
    cdef _rpc_controller *thisptr
    cdef _sync_event *sync_event

    def __cinit__(self):
        self.thisptr = new _rpc_controller()
        self.sync_event = new _sync_event()
    def __dealloc__(self):
        del self.sync_event
        del self.thisptr
    def ok(self):
        return self.thisptr.ok()
    def wait(self):
        with nogil:
            self.sync_event.wait()

    property status:
        def __get__(self):
            return self.thisptr.get_status()
    property application_error_code:
        def __get__(self):
            return self.thisptr.get_application_error_code()
    property error_message:
        def __get__(self):
            return string_to_pystring(self.thisptr.get_error_message())
    property deadline_ms:
        def __get__(self):
            return self.thisptr.get_deadline_ms()
        def __set__(self, value):
            self.thisptr.set_deadline_ms(value)


cdef struct ClosureWrapper:
    string* response_str
    void* response_obj
    void* callback
    void* rpc_controller


cdef extern from "rpcz/callback.hpp" namespace "rpcz":
    cdef cppclass closure:
        pass

    closure* new_callback(void(ClosureWrapper*) nogil, ClosureWrapper*)


# this function is called from C++ after we gave up the GIL. We use "with gil"
# to acquire it.
cdef void python_callback_bridge(ClosureWrapper *closure_wrapper) with gil:
    (<object>closure_wrapper.response_obj).ParseFromString(
            string_ptr_to_pystring(closure_wrapper.response_str))
    response = <object>closure_wrapper.response_obj;
    callback = <object>closure_wrapper.callback
    rpc_controller = <RpcController>closure_wrapper.rpc_controller
    rpc_controller.sync_event.signal()
    if callback is not None:
        callback(response, rpc_controller)
    Py_DECREF(<object>closure_wrapper.response_obj)
    Py_DECREF(<object>closure_wrapper.callback)
    Py_DECREF(<object>closure_wrapper.rpc_controller)
    del closure_wrapper.response_str
    free(closure_wrapper)


cdef extern from "rpcz/rpc_channel.hpp" namespace "rpcz":
    cdef cppclass _rpc_channel "rpcz::rpc_channel":
        void call_method0(string service_name, string method_name,
                          string request, string* response,
                          _rpc_controller* rpc_controller, 
                          closure* callback) except +
    _rpc_channel * rpc_channel_create "rpcz::rpc_channel::create" (string endpoint)

cdef class RpcChannel:
    cdef _rpc_channel *thisptr
    def __dealloc__(self):
        del self.thisptr
    def __init__(self):
        raise TypeError("Use Application.create_rpc_channel to create a "
                        "RpcChannel.")
    def call_method(self, service_name, method_name,
                   request, response, RpcController rpc_controller, callback):
        cdef ClosureWrapper* closure_wrapper = <ClosureWrapper*>malloc(
                sizeof(ClosureWrapper))
        closure_wrapper.response_str = new string()
        closure_wrapper.response_obj = <void*>response
        closure_wrapper.callback = <void*>callback
        closure_wrapper.rpc_controller = <void*>rpc_controller
        Py_INCREF(response)
        Py_INCREF(callback)
        Py_INCREF(rpc_controller)
        self.thisptr.call_method0(
                make_string(service_name),
                make_string(method_name),
                make_string(request.SerializeToString()),
                closure_wrapper.response_str,
                rpc_controller.thisptr,
                new_callback(
                    python_callback_bridge, closure_wrapper))


cdef extern from "rpcz/replier.hpp" namespace "rpcz":
  cdef cppclass _replier "rpcz::replier":
    _replier(_replier)
    void send_error(int, string)
    void send0(string)
 

cdef class Replier:
    cdef _replier *thisptr
    def __init__(self):
        raise TypeError("Do not initialize directly.")
    def __dealloc__(self):
        del self.thisptr
    def send_error(self, application_error_code, error_string=""):
      self.thisptr.send_error(application_error_code,
                             make_string(error_string))
      del self.thisptr
      self.thisptr = NULL
    def send(self, message):
      self.thisptr.send0(make_string(message.SerializeToString()))
      del self.thisptr
      self.thisptr = NULL


ctypedef void(*Handler)(object user_service, string method,
                        void* payload, size_t payload_len,
                        _replier replier_copy) nogil


cdef void rpc_handler_bridge(object user_service, string& method,
                             void* payload, size_t payload_len,
                             _replier replier_copy) with gil:
  cdef Replier replier_ = Replier.__new__(Replier)
  replier_.thisptr = new _replier(replier_copy)
  user_service._call_method(string_to_pystring(method),
                         cstring_to_pystring(payload, payload_len),
                         replier_)


cdef extern from "python_rpc_service.hpp" namespace "rpcz":
    cdef cppclass PythonRpcService:
        PythonRpcService(Handler, object)


cdef extern from "rpcz/application.hpp" namespace "rpcz::application":
    void terminate()
    void run() nogil
    void set_connection_manager_threads(int n)
    void set_zmq_io_threads(int n)


def create_rpc_channel_wrap(endpoint):
    cdef RpcChannel channel = RpcChannel.__new__(RpcChannel)
    channel.thisptr = rpc_channel_create(make_string(endpoint))
    return channel

def terminate_wrap():
    terminate()

def run_wrap():
    with nogil:
        run()

def set_connection_manager_threads_wrap(n):
    set_connection_manager_threads(n)

def set_zmq_io_threads_wrap(n):
    set_zmq_io_threads(n)

cdef extern from "rpcz/server.hpp" namespace "rpcz":
    cdef cppclass _server "rpcz::server":
        void register_rpc_service(PythonRpcService*, string name)
        void bind(string endpoint)


cdef class Server:
    cdef _server *thisptr
    def __cinit__(self):
        self.thisptr = new _server()
    def __dealloc__(self):
        del self.thisptr
    def register_service(self, service, name=None):
        cdef PythonRpcService* rpc_service = new PythonRpcService(
            rpc_handler_bridge, service)
        self.thisptr.register_rpc_service(rpc_service, make_string(name))
    def bind(self, endpoint):
        self.thisptr.bind(make_string(endpoint))


