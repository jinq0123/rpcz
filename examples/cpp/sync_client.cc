// Copyright 2011 Google Inc. All Rights Reserved.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Author: nadavs@google.com <Nadav Samet>

#include <iostream>

#include "cpp/search.rpcz.h"

using namespace std;

int main() {
  examples::SearchService_Stub search_stub("tcp://localhost:5555");
  examples::SearchRequest request;
  request.set_query("gold");

  cout << "Sending request." << endl;
  try {
    examples::SearchResponse response
        = search_stub.Search(request, 1000);
    cout << response.DebugString() << endl;
  } catch (rpcz::rpc_error& e) {
    cout << "Error: " << e.what() << endl;;
  }
}
