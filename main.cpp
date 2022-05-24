#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <assert.h>

#ifndef _ASSERT_M
#define assertm(exp, msg) assert(((void)msg, exp));
#endif

#ifndef _BUFFER_SIZE
#define BUFFER_SIZE 255
#endif

#ifndef _LOOKAHEAD_SIZE
#define LOOKAHEAD_SIZE 15
#endif

/* *
 * LZSS Compressor
 * by baboomerang - March 30th 2022
 *
 * This program does something, and does something else. This is just filler text
 * using a template file. Replace this description with something descriptive about
 * the project.
 *
 * */

struct Triplet {
    unsigned offset;
    unsigned length;
    char unmatched_token;
};

//template<typename InputIt, typename UnaryPredicate>
//typename iterator_traits<InputIt>::difference_type
//count_if_on_two_ranges(InputIt first, InputIt last,
//                       InputIt first2, InputIt last2, UnaryPredicate p) {
//
//    typename iterator_traits<InputIt>::difference_type count{0};
//    for (; (first != last) && (first2 != last2); ++first, ++first2) {
//        if (p(*first, *first2)) {
//            ++count;
//        }
//    }
//
//    return count;
//}

// find one continuous run of matching values between two ranges
template<class InputIt, class UnaryPredicate>
InputIt consecutive_match_run(InputIt first, InputIt last,
                InputIt first2, InputIt last2, UnaryPredicate p) {

    InputIt result = last;
    for (; (first != last) && (first2 != last2); ++first, ++first2) {
        if (p(*first, *first2)) {
            result = first;
        } else {
            break;
        }
    }
    return result;
}




int encode_lz77(std::ifstream& input, std::ofstream& output) {
    // get token from ifstream
    // search for token in buffer
    // if match, get index of the first match, increment length, LOOP
    //     there are multiple possible match patterns, so find the longest match of the bunch
    //
    // if not match, leave the triplet as default (0,0, token)
    // write the triplet to the output file
    // (what if the matches are larger than 255)

    std::vector<char> search_buffer;
    std::vector<char> look_buffer;
    char token;

    for (auto x{0}; x < LOOKAHEAD_SIZE; ++x) {
        if (input.get(token)) {
            look_buffer.push_back(token);
        }
    }


    auto pos = consecutive_match_run(search_buffer.begin(), search_buffer.end(),
                                     look_buffer.begin(), look_buffer.end(),
                                     [](char dictionary, char token) {
                                        return dictionary == token;
                                     });

    auto length = std::distance()
    return 0;
}

int decode_lz77(std::ifstream& input, std::ofstream& output) {

    while (input) {

    }

    return 0;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <file>\n";
        return 1;
    }

    const std::string filename = argv[1];
    const std::string new_filename = filename + ".lz77";

    std::ifstream input{filename, std::ios_base::binary};
    assertm(input.is_open(), (filename + " cannot read file."));

    std::ofstream output{new_filename, std::ios_base::binary};
    assertm(output.is_open(), (new_filename + " cannot write file."));


    std::cout << "File successfully compressed to: " << new_filename << std::endl;

    return 0;
}
