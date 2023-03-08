#define NONIUS_RUNNER
#include <nonius/nonius_single.h++>
#include <async_json/json_extractor.hpp>
#include <ranges>

char const model_data[] = R"(
{
  "kind": "youtube#searchListResponse",
  "etag": "\"m2yskBQFythfE4irbTIeOgYYfBU/PaiEDiVxOyCWelLPuuwa9LKz3Gk\"",
  "nextPageToken": "CAUQAA",
  "regionCode": "KE",
  "pageInfo": {
    "totalResults": 4249,
    "resultsPerPage": 5
  },
  "items": [
    {
      "kind": "youtube#searchResult",
      "etag": "\"m2yskBQFythfE4irbTIeOgYYfBU/QpOIr3QKlV5EUlzfFcVvDiJT0hw\"",
      "id": {
        "kind": "youtube#channel",
        "channelId": "UCJowOS1R0FnhipXVqEnYU1A"
      }
    },
    {
      "kind": "youtube#searchResult",
      "etag": "\"m2yskBQFythfE4irbTIeOgYYfBU/sbCCQ5dWx1zCQwp5fKHlrtFTFJ4\"",
      "id": {
        "kind": "youtube#video",
        "videoId": "IGYEtw94zMM"
      }
    },
    {
      "kind": "youtube#searchResult",
      "etag": "\"m2yskBQFythfE4irbTIeOgYYfBU/dKYVYE72ZdmqgW9jTeTQOLtgUsI\"",
      "id": {
        "kind": "youtube#video",
        "videoId": "nxa6TeeJQ1s"
      }
    },
    {
      "kind": "youtube#searchResult",
      "etag": "\"m2yskBQFythfE4irbTIeOgYYfBU/AWutzVOt_5p1iLVifyBdfoSTf9E\"",
      "id": {
        "kind": "youtube#video",
        "videoId": "Eqa2nAAhHN0"
      }
    },
    {
      "kind": "youtube#searchResult",
      "etag": "\"m2yskBQFythfE4irbTIeOgYYfBU/2dIR9BTfr7QphpBuY3hPU-h5u-4\"",
      "id": {
        "kind": "youtube#video",
        "videoId": "IirngItQuVs"
      }
    }
  ],
  "result": {
    "kind": "youtube#searchListResponse",
    "etag": "\"m2yskBQFythfE4irbTIeOgYYfBU/PaiEDiVxOyCWelLPuuwa9LKz3Gk\"",
    "nextPageToken": "CAUQAA",
    "regionCode": "KE",
    "pageInfo": {
      "totalResults": 4249,
      "resultsPerPage": 5
    },
    "items": [
      {
        "kind": "youtube#searchResult",
        "etag": "\"m2yskBQFythfE4irbTIeOgYYfBU/QpOIr3QKlV5EUlzfFcVvDiJT0hw\"",
        "id": {
          "kind": "youtube#channel",
          "channelId": "UCJowOS1R0FnhipXVqEnYU1A"
        }
      },
      {
        "kind": "youtube#searchResult",
        "etag": "\"m2yskBQFythfE4irbTIeOgYYfBU/sbCCQ5dWx1zCQwp5fKHlrtFTFJ4\"",
        "id": {
          "kind": "youtube#video",
          "videoId": "IGYEtw94zMM"
        }
      },
      {
        "kind": "youtube#searchResult",
        "etag": "\"m2yskBQFythfE4irbTIeOgYYfBU/dKYVYE72ZdmqgW9jTeTQOLtgUsI\"",
        "id": {
          "kind": "youtube#video",
          "videoId": "nxa6TeeJQ1s"
        }
      },
      {
        "kind": "youtube#searchResult",
        "etag": "\"m2yskBQFythfE4irbTIeOgYYfBU/AWutzVOt_5p1iLVifyBdfoSTf9E\"",
        "id": {
          "kind": "youtube#video",
          "videoId": "Eqa2nAAhHN0"
        }
      },
      {
        "kind": "youtube#searchResult",
        "etag": "\"m2yskBQFythfE4irbTIeOgYYfBU/2dIR9BTfr7QphpBuY3hPU-h5u-4\"",
        "id": {
          "kind": "youtube#video",
          "videoId": "IirngItQuVs"
        }
      }
    ]
  }
}
)";

NONIUS_BENCHMARK("parse_with_tables",
                 []
                 {
                     int         kind_counter;
                     int         items_counter;
                     int         total_results;
                     std::string next_page_token;
                     std::string channel_id;
                     std::string etag;
                     using namespace async_json;
                     auto parser = make_extractor(                            //
                         [](auto error) { std::cerr << "It failed\n"; },      //
                         path([](auto const&) {}, "nextPageToken"),           //
                         path(                                                //
                             all(                                             //
                                 path([](auto const&) {}, "etag"),            //
                                 path([](auto const&) {}, "id", "channelId")  //
                                 ),                                           //
                             "result", "items")                               //
                     );
                     parser.parse_bytes({model_data, sizeof(model_data)});
                 })

NONIUS_BENCHMARK("parse_with_unrolled",
                 []
                 {
                     int         kind_counter;
                     int         items_counter;
                     int         total_results;
                     std::string next_page_token;
                     std::string channel_id;
                     std::string etag;
                     using namespace async_json;
                     auto parser =
                         make_fast_extractor([](auto error) { std::cerr << "It failed\n"; },
                                             fast_path([](auto const&) { std::cerr << "token recevied\n"; }, "nextPageToken"),  //
                                             fast_path(                                                                         //
                                                 all(                                                                           //
                                                     fast_path([](auto const&) {}, "etag"),                                     //
                                                     fast_path([](auto const&) { std::cerr << "found\n"; }, "id", "channelId")  // a
                                                     ),
                                                 "result", "items")  //
                         );
                     parser.parse_bytes({model_data, sizeof(model_data)});
                 })

