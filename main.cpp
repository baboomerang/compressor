/* *
 * LZSS Compressor
 * by baboomerang - March 30th 2022
 *
 * This program does something, and does something else. This is just filler text
 * using a template file. Replace this description with something descriptive about
 * the project.
 *
 * Please remember that this code is by no means optimal, nor is it intended
 * to be used for any data compression purposes.
 * */

#include <cstddef>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>
#include <limits>
#include <exception>
#include <filesystem>
#include <getopt.h>

#ifndef _BUFFER_SIZE
#define BUFFER_SIZE 240
#endif

#ifndef _LOOKAHEAD_SIZE
#define LOOKAHEAD_SIZE 15
#endif

static std::vector<unsigned char> window;

// Description:
//     Find the first mismatch between two ranges
// Parameters:
//     first1, last1 - the primary range to check
//     first2, last2 - the secondary range to check
// Return Value:
//     Pair of Iterators to the first occurence where both ranges do not share the same value
//     if both ranges are the same (no mismatches found), then (last1, last2) is returned
//
//template<class ForwardIterator1, class ForwardIterator2>
//std::pair<ForwardIterator1, ForwardIterator2>
//    mismatch(ForwardIterator1 first1, ForwardIterator1 last1,
//             ForwardIterator2 first2, ForwardIterator2 last2) {
//
//    while (first1 != last1 && first2 != last2 && *first1 == *first2) {
//        ++first1, ++first2;
//    }
//
//    return std::make_pair(first1, first2);
//}

// Description:
//     Find the largest ordered subset between two unordered sets
// Parameters:
//     first1, last1 - the range of elements to examine
//     first2, last2 - the range of elements to search for
// Return Value:
//     Pair of Iterators to the largest partial or whole occurence of the sequence [first2, last2) in the range [first1, last1)
//     if no such occurence is found, (last1, last1) is returned
//
// This function is similar to std::search, but std::search only finds a whole, exact match
// So the match length is assumed constant and only needs to return 1 iterator
//
// In the case for LZ77, there are partial matches too
// So this function needs to return the beginning of the sequence, and the match length (as a scalar or second iterator)
//
template<class ForwardIterator1, class ForwardIterator2>
std::pair<ForwardIterator1, ForwardIterator1>
    find_longest_match(ForwardIterator1 first1, ForwardIterator1 last1,
                       ForwardIterator2 first2, ForwardIterator2 last2) {

    using iterator1_traits = typename std::iterator_traits<ForwardIterator1>;

    ForwardIterator1 result = last1;
    typename iterator1_traits::difference_type best_length{0};
    typename iterator1_traits::difference_type best_offset{0};
    for (; (first1 != last1); ++first1) {
        const std::pair<ForwardIterator1, ForwardIterator2> p{std::mismatch(first1, last1, first2, last2)};
        const typename iterator1_traits::difference_type length{std::distance(first1, p.first)};
        const typename iterator1_traits::difference_type offset{std::distance(first1, last1)};

        if (length > best_length || (length == best_length && offset < best_offset)) {
            best_length = length;
            best_offset = offset;
            result = first1;
        }
    }

    return std::make_pair(result, std::next(result, best_length));
}

std::size_t get_streamsize(std::ifstream& istr) {
    // save the current position of the cursor
    const std::streampos pos{istr.tellg()};

    // seek to the end of the stream
    istr.ignore(std::numeric_limits<std::streamsize>::max());
    std::size_t size{static_cast<std::size_t>(istr.gcount())};

    // move file pointer back to the beginning of the stream
    istr.seekg(pos, std::ios_base::beg);

    // clear eof bit
    istr.clear();

    return size;
}

void encode_lz77(std::ifstream& input, std::ofstream& output) {
    // get size of input stream
    const std::size_t filesize{get_streamsize(input)};

    // preallocate maximum vector size
    window.reserve(filesize);

    // read entire file into a global vector
    input.read(reinterpret_cast<std::ifstream::char_type*>(&window.front()), filesize);

    // allocate the initial size of the lookahead buffer
    const std::size_t initial_size{(filesize >= LOOKAHEAD_SIZE) ? LOOKAHEAD_SIZE : filesize};
    std::size_t bytes_left{filesize - initial_size};
    std::vector<unsigned char>::iterator search{window.begin()};
    std::vector<unsigned char>::iterator look{window.begin()};
    std::vector<unsigned char>::iterator look_end{std::next(look, initial_size)};

    while (look != look_end) {
        std::cout << search - window.begin() << " " << look - window.begin() << " "
            << look_end - window.begin() << '\n';

        // if there are no bytes left, include the lookahead buffer in the search range
        const auto& search_end = (bytes_left) ? look : look_end;
        const auto result = find_longest_match(search, search_end, look, look_end);
        const auto match_length{std::distance(result.first, result.second)};
        const auto match_offset{std::distance(result.first, look)};

        if (match_length < 0) {
            throw std::range_error("match_length has unexpected negative value!");
        }

        if (match_offset < 0) {
            throw std::range_error("match_offset has unexpected negative value!");
        }

        // lz77 uses the token after the matched sequence
        const unsigned char token{*(std::next(look, match_length))};

        output << static_cast<unsigned char>(match_offset)
               << static_cast<unsigned char>(match_length)
               << token;

        // shift cursor (look) by match_length + 1 as required by lz77 algorithm
        // EXCEPT at the end of the file, the final shift must be match_length
        // this prevents (look) from incrementing +1 past (look_end) at the end
        const auto shift_length{(bytes_left) ? match_length + 1 : match_length};
        const auto available_length{(bytes_left >= shift_length) ? shift_length : bytes_left};
        std::advance(look, shift_length);
        std::advance(look_end, available_length);

        // shrink the search buffer if it is larger than BUFFER_SIZE
        const auto distance{std::distance(search, look)};
        if (distance > BUFFER_SIZE) {
            std::advance(search, distance - BUFFER_SIZE);
        }

        // safety check since bytes_left is an unsigned type and could underflow
        if (available_length > bytes_left) {
            throw std::range_error("available_length has an unexpected value larger than the filesize!");
        }

        bytes_left -= available_length;
    }
}

void decode_lz77(std::ifstream& input, std::ofstream& output) {
    // get size of input stream
    const std::size_t filesize{get_streamsize(input)};

    // preallocate maximum vector size
    window.reserve(filesize);

    // read entire file into a global vector
    input.read(reinterpret_cast<std::ifstream::char_type*>(&window.front()), filesize);

    // TODO: allocate the sliding window,
    //       characters shifted out of the window should be written to the output file


}

void print_usage(std::string program_name) {
    std::cerr << "Usage: " << program_name << " <option(s)>\n"
        << "Options:\n"
        << "\t-h,--help\t\tShow this help message\n"
        << "\t-d,--decode FILENAME\tDecode the file\n"
        << "\t-e,--encode FILENAME\tEncode the file\n"
        << std::endl;
}

bool file_exists(const std::filesystem::path& p,
                 std::filesystem::file_status s = std::filesystem::file_status()) {

    return (std::filesystem::status_known(s) ? std::filesystem::exists(s)
                                             : std::filesystem::exists(p));
}

int main(int argc, char** argv) {
    static const std::string program_name{argv[0]};

    if (argc < 2) {
        std::cerr << "Error: this program requires an input file\n";
        print_usage(program_name);
        return 1;
    }

    int decompress_flag{0};
    std::string filename;
    std::string output_filename;
    while (true) {
        static struct option long_options[] = {
            {"encode", no_argument, &decompress_flag, 0},
            {"decode", no_argument, &decompress_flag, 1},
            {"help", no_argument, 0, 'h'},
            {"outputfile", required_argument, 0, 'o'},
            {"inputfile",  required_argument, 0, 'i'},
            {0, 0, 0, 0}
        };

        int option_index{0};
        int opt{getopt_long(argc, argv, "hi:o:", long_options, &option_index)};

        if (opt == -1) {
            break;
        }

        switch(opt) {
            case 'i':
                filename.assign(optarg);
                break;
            case 'o':
                output_filename.assign(optarg);
                break;
            case 'h':
                print_usage(program_name);
                return 1;
            case '?':
                std::cerr << "Error: invalid command argument(s) " << optarg << '\n';
                return 1;
            default:
                // normally there would be an abort() here
                // this return statement is similar
                return 1;
        }
    }

    // set default output name if not explicitly set in the above switch case
    if (output_filename.empty()) {
        output_filename = filename + ".lz77";
    }

    const std::filesystem::path input_filepath{filename};
    if (!file_exists(input_filepath)) {
        std::cerr << "Error: cannot read file. " << filename << " does not exist or is not readable.\n";
        return 1;
    }

    const std::filesystem::path output_filepath{output_filename};
    if (file_exists(output_filepath)) {
        char choice;
        std::cout << "File " << output_filename << " exists. Overwrite file? [y/N]: ";
        std::cin >> choice;
        if (choice == 'y' || choice == 'Y') {
            std::filesystem::remove(output_filepath);
        } else {
            return 1;
        }
    }

    std::ifstream input{filename, std::ios_base::binary};
    std::ofstream output{output_filename, std::ios_base::binary};

    try {
        switch(decompress_flag) {
            case 0:
                encode_lz77(input, output);
                std::cout << "File succesfully compressed to: " << output_filename << '\n';
                break;
            case 1:
                decode_lz77(input, output);
                std::cout << "File succesfully decompressed to: " << output_filename << '\n';
                break;
            default:    // this case should ideally never happen, UNLESS there's a soft error
                throw std::runtime_error("decompress_flag has unexpected value neither 1 or 0");
        }
    } catch (std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}

//for (auto search_it = search.begin(); search_it != search.end(); ++search_it) {
//    auto result = find_match_run(search_it, search.end(),
//                                 look.begin(), look.end(),
//                                 [](auto dict, auto token){return dict == token;});

//    if (result == search.end()) {
//        continue;
//    }

//    auto length = std::distance(search_it, result);
//    auto offset = std::distance(search_it, search.end());

//    if (length > data.length || (length == data.length && offset < data.offset)) {
//        data.length = length;
//        data.offset = offset;
//        data.token = *result;
//    }
//}
