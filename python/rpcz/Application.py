import pywraprpcz

def create_rpc_channel(endpoint):
    return pywraprpcz.create_rpc_channel_wrap(endpoint)
    
def terminate():
    pywraprpcz.terminate()
    
def run():
    pywraprpcz.run()

def set_connection_manager_threads(count):
    pywraprpcz.set_connection_manager_threads(count)

def set_zmq_io_threads(count):
    pywraprpcz.set_zmq_io_threads(count)
    
def run():
    pywraprpcz.run()
