#!/usr/bin/env python

import rpcz
import search_pb2
import search_rpcz

class SearchService(search_rpcz.SearchService):
  def Search(self, request, reply):
    print "Got request for '%s'" % request.query
    response = search_pb2.SearchResponse()
    response.results.append("result1 for " + request.query)
    response.results.append("this is result2")
    reply.send(response)

server = rpcz.Server()
server.register_service(SearchService(), "SearchService")
server.bind("tcp://*:5555")
print "Serving requests on port 5555"
rpcz.Application.run()
