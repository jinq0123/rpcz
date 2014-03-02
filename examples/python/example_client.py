#!/usr/bin/env python

import rpcz
import search_pb2
import search_rpcz

stub = search_rpcz.SearchService_Stub(
        rpcz.Application.create_rpc_channel("tcp://127.0.0.1:5555"))

request = search_pb2.SearchRequest()
request.query = 'gold'
print stub.Search(request, deadline_ms=1000)
