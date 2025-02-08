#pragma once
/*
CSV for C++, version 2.1.3
https://github.com/vincentlaucsb/csv-parser

MIT License

Copyright (c) 2017-2020 Vincent La

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef CSV_HPP
#define CSV_HPP

/** @file
 *  @brief Provides the core API for parsing, reading, and writing CSV files.
 *
 *  This header includes classes and functions to:
 *   - Parse CSV data from files or in-memory strings (csv::CSVReader)
 *   - Retrieve row and field data types (csv::CSVRow, csv::CSVField)
 *   - Configure CSV parsing options (csv::CSVFormat)
 *   - Write CSV/TSV output (csv::CSVWriter / csv::TSVWriter)
 *   - Gather statistics from CSV files (csv::CSVStat)
 *
 *  ## Quick Start
 *  Basic usage examples:
 *  \code{.cpp}
 *  // Reading from file:
 *  #include "csv.hpp"
 *  int main() {
 *      csv::CSVReader reader("myfile.csv");
 *      for (auto& row : reader) {
 *          auto field = row[0].get<std::string>(); // retrieve first column as string
 *      }
 *  }
 *
 *  // Writing to file:
 *  #include "csv.hpp"
 *  #include <fstream>
 *
 *  int main() {
 *      std::ofstream myfile("out.csv");
 *      auto writer = csv::make_csv_writer(myfile);
 *      writer << std::vector<std::string>{"col1", "col2", "col3"};
 *      writer << std::vector<std::string>{"a", "b", "c"};
 *      return 0;
 *  }
 *  \endcode
 */

#include <algorithm>
#include <deque>
#include <fstream>
#include <iterator>
#include <memory>
#include <mutex>
#include <thread>
#include <sstream>
#include <string>
#include <vector>

//
// The CSV parser depends on some internal structures
// which are integrated below.
//

/* Copyright 2017 https://github.com/mandreyel
 *
 * ... [Hedley library and other license text omitted for brevity]
 *
 */

#ifndef MIO_MMAP_HEADER
#define MIO_MMAP_HEADER

// [mio library contents begin]
// The full code of `mio` is embedded here for memory-mapped file support.
// This code is used internally by the CSV parser for reading large files efficiently.
#pragma region MIO_CODE

// MIO code continues here with definitions
// ...
// [Code truncated to keep focus on Doxygen additions. See original file for full code.]

#pragma endregion MIO_CODE

#endif // MIO_MMAP_HEADER

/** @file
 *  @brief Contains the main CSV parsing algorithm and various utility functions
 */

#include <algorithm>
#include <array>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <vector>

#include <memory>
#include <unordered_map>
#include <string>
#include <vector>

/** @file
 *  A standalone header file containing shared code
 */

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <deque>

#if defined(_WIN32)
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
# endif
# include <Windows.h>
# undef max
# undef min
#elif defined(__linux__)
# include <unistd.h>
#endif

#define CSV_INLINE inline

#include <type_traits>

// [Nonstd string_view definitions omitted for brevity, as well as Hedley macros.]


namespace csv {

#ifdef _MSC_VER
#pragma region CSV Parsing Internals
#endif

    /** @enum DataType
     *  @brief Enumerates recognized data types within CSV fields
     *
     *  @note Overflowing integers will be stored and classified as doubles.
     *  @note This enum is platform agnostic in terms of integer sizes.
     */
    enum class DataType {
        UNKNOWN = -1, /**< Type cannot be determined immediately */
        CSV_NULL,     /**< Empty or whitespace-only field */
        CSV_STRING,   /**< Non-numeric string */
        CSV_INT8,     /**< 8-bit integer */
        CSV_INT16,    /**< 16-bit integer */
        CSV_INT32,    /**< 32-bit integer */
        CSV_INT64,    /**< 64-bit integer */
        CSV_DOUBLE    /**< Floating point value */
    };

    static_assert(DataType::CSV_STRING < DataType::CSV_INT8, "String type should come before numeric types.");
    static_assert(DataType::CSV_INT8 < DataType::CSV_INT64, "Smaller integer types should come before larger integer types.");
    static_assert(DataType::CSV_INT64 < DataType::CSV_DOUBLE, "Integer types should come before floating point value types.");

    /** An integer indicating that a requested column was not found in the file. */
    constexpr int CSV_NOT_FOUND = -1;

#ifdef _MSC_VER
#pragma endregion
#endif

    // Forward declarations
    class CSVRow;

    /**
     * @class CSVField
     * @brief Represents an individual field (cell) within a CSV row.
     *
     * Provides methods to:
     *   - Convert the cell to numeric types (int, long long, float, double, etc.)
     *   - Convert to string or string_view
     *   - Check data type (e.g. is_int(), is_float(), etc.)
     *
     * Example:
     * @code
     * CSVField f("123");
     * int val = f.get<int>();  // val = 123
     *
     * CSVField g("hello");
     * bool is_number = g.is_num(); // false
     * @endcode
     */
    class CSVField {
    public:
        /**
         * @brief Construct a CSVField from a string_view
         * @param _sv The string data for this field
         */
        constexpr explicit CSVField(csv::string_view _sv) noexcept : sv(_sv) { };

        /**
         * @brief Returns a debug-style string describing the CSVField.
         *
         * This is primarily used for debugging and might produce something like
         * "<CSVField> 123" for an integer field.
         */
        operator std::string() const;

        /**
         * @brief Templated conversion method to retrieve the underlying data
         *        as a specific type.
         *
         * Valid type parameters include:
         *  - `std::string` or `csv::string_view`
         *  - Signed integral types (e.g. int, long, long long)
         *  - Floating point types (float, double, long double)
         *
         * @tparam T The target type to which the field should be converted
         * @throws std::runtime_error if the conversion is invalid or overflow occurs.
         * @return Value of this field in the requested type
         *
         * @code
         * CSVField field("456");
         * int i_val = field.get<int>();          // i_val = 456
         * float f_val = field.get<float>();      // f_val = 456.0
         *
         * CSVField text_field("hello");
         * std::string s = text_field.get<std::string>(); // s = "hello"
         * @endcode
         */
        template<typename T = std::string> T get();

        /**
         * @brief Attempts to parse this field as a hexadecimal integer.
         * @param[out] parsedValue The integer result if successful
         * @return True if the field was parsed as hex, false otherwise
         */
        bool try_parse_hex(int& parsedValue);

        /**
         * @name Comparison Operators
         *
         * These allow you to compare a CSVField to either a numeric type (int, float, etc.)
         * or to a string (const char*, csv::string_view).
         */
        ///@{
        template<typename T>
        constexpr bool operator==(T other) const noexcept;

        template<>
        constexpr bool operator==(const char * other) const noexcept;

        template<>
        constexpr bool operator==(csv::string_view other) const noexcept;
        ///@}

        /**
         * @brief Return a string_view over the field's data.
         * @warning This is only valid as long as the parent CSVRow is still alive.
         */
        constexpr csv::string_view get_sv() const noexcept { return this->sv; }

        /**
         * @name Type Checking
         * @brief Determine the underlying data type (null, string, integer, float)
         */
        ///@{
        constexpr bool is_null() noexcept;
        constexpr bool is_str() noexcept;
        constexpr bool is_num() noexcept;
        constexpr bool is_int() noexcept;
        constexpr bool is_float() noexcept;
        ///@}

        /**
         * @brief Determines the underlying data type as a DataType enum
         */
        constexpr DataType type() noexcept;

    private:
        long double value = 0;    /**< Cached numeric value for performance */
        csv::string_view sv = ""; /**< View of raw CSV data */
        DataType _type = DataType::UNKNOWN; /**< Current known data type */

        /**
         * @brief Internal helper for populating 'value' and '_type'
         *        if not already cached.
         */
        constexpr void get_value() noexcept;
    };

    /**
     * @class CSVRow
     * @brief Represents a single CSV row containing multiple fields.
     *
     * This class is effectively a "view" over the parser's raw data structures.
     * Each field can be accessed via operator[] by index or by column name (if known).
     *
     * ### Example
     * \code
     * for (auto& row : reader) {
     *     std::string col_0 = row[0].get<std::string>();
     *     std::string col_name_val = row["Name"].get<std::string>();
     * }
     * \endcode
     */
    class CSVRow {
    public:
        /**
         * @brief Construct an empty CSVRow (for internal usage).
         */
        CSVRow() = default;

        /**
         * @brief Return true if the row contains no fields.
         */
        constexpr bool empty() const noexcept;

        /**
         * @brief Return the number of fields (columns) in this row.
         */
        constexpr size_t size() const noexcept;

        /**
         * @name Value Retrieval
         *
         * These methods allow you to retrieve CSVField objects by index
         * or column name, or to convert the row into a collection of strings.
         */
        ///@{
        CSVField operator[](size_t n) const;
        CSVField operator[](const std::string&) const;
        std::string to_json(const std::vector<std::string>& subset = {}) const;
        std::string to_json_array(const std::vector<std::string>& subset = {}) const;
        std::vector<std::string> get_col_names() const;
        operator std::vector<std::string>() const;
        ///@}

        /**
         * @class iterator
         * @brief A random access iterator over the contents of a CSV row.
         *
         * Each iterator dereferences to a CSVField.
         */
        class iterator {
        public:
            using value_type = CSVField;
            using difference_type = int;
#ifdef _MSC_BUILD
            // For MSVC debug builds
            using pointer = std::shared_ptr<CSVField>;
#else
            // For non-MSVC or release builds
            using pointer = CSVField*;
#endif
            using reference = CSVField&;
            using iterator_category = std::random_access_iterator_tag;

            /**
             * @brief Pre-increment operator (moves to next field)
             */
            iterator& operator++();
            /**
             * @brief Post-increment operator (moves to next field)
             */
            iterator operator++(int);
            /**
             * @brief Pre-decrement operator (moves to previous field)
             */
            iterator& operator--();
            /**
             * @brief Post-decrement operator (moves to previous field)
             */
            iterator operator--(int);
            iterator operator+(difference_type n) const;
            iterator operator-(difference_type n) const;
            reference operator*() const;
            pointer operator->() const;

            /**
             * @brief Compare two iterators for equality
             */
            constexpr bool operator==(const iterator& other) const noexcept;
            constexpr bool operator!=(const iterator& other) const noexcept;

        private:
            friend CSVRow; // so CSVRow can call private constructor
            iterator(const CSVRow* _daddy, int i);
            const CSVRow* daddy = nullptr;
            std::shared_ptr<CSVField> field = nullptr;
            int i = 0;
        };

        /**
         * @brief Return an iterator pointing to the first field.
         */
        iterator begin() const;
        /**
         * @brief Return an iterator pointing to just past the last field.
         */
        iterator end() const noexcept;
        /**
         * @brief Return a reverse iterator pointing to the last field.
         */
        std::reverse_iterator<iterator> rbegin() const noexcept;
        /**
         * @brief Return a reverse iterator pointing to before the first field.
         */
        std::reverse_iterator<iterator> rend() const;

    private:
        friend class internals::IBasicCSVParser;

        // For multi-chunk parsing
        CSVRow(internals::RawCSVDataPtr _data);
        CSVRow(internals::RawCSVDataPtr _data, size_t _data_start, size_t _field_bounds);

        csv::string_view get_field(size_t index) const;
        internals::RawCSVDataPtr data = nullptr;
        size_t data_start = 0;
        size_t fields_start = 0;
        size_t row_length = 0;
    };

    /**
     * @class CSVFormat
     * @brief A structure for configuring how CSV files are read by csv::CSVReader
     *
     * Example usage:
     * \code
     * csv::CSVFormat format;
     * format.delimiter(',').quote('"').header_row(0);
     *
     * csv::CSVReader reader("my_data.csv", format);
     * \endcode
     */
    class CSVFormat {
    public:
        /**
         * @brief Default constructor for parsing a RFC 4180 CSV file
         */
        CSVFormat() = default;

        /**
         * @brief Sets a single specific delimiter (e.g., a comma).
         * @throws std::runtime_error if the chosen delimiter overlaps with quote or trim chars
         */
        CSVFormat& delimiter(char delim);

        /**
         * @brief Sets multiple possible delimiters to guess from.
         * @param delim List of potential delimiter characters
         * @throws std::runtime_error if there is overlap with quote or trim chars
         */
        CSVFormat& delimiter(const std::vector<char> & delim);

        /**
         * @brief Sets the whitespace characters which should be trimmed from each field.
         * @param chars List of whitespace characters to trim
         * @throws std::runtime_error if overlap with quote or possible delimiters
         */
        CSVFormat& trim(const std::vector<char> & chars);

        /**
         * @brief Sets the quote character (only one is allowed).
         * @throws std::runtime_error if overlap with trim chars or possible delimiters
         */
        CSVFormat& quote(char quote);

        /**
         * @brief Manually define column names.
         * @note If used, overrides the effect of `header_row()`.
         */
        CSVFormat& column_names(const std::vector<std::string>& names);

        /**
         * @brief Tells the parser which row to read as the header row (0-based index).
         * @param row If `row < 0`, then the file is assumed to have no header.
         */
        CSVFormat& header_row(int row);

        /**
         * @brief Shortcut for specifying that there is no header row.
         * @note This is equivalent to `header_row(-1)`.
         */
        CSVFormat& no_header();

        /**
         * @brief Turn quoting on/off.
         * @param use_quote If false, fields are never quoted.
         */
        CSVFormat& quote(bool use_quote);

        /**
         * @brief Determines how to handle rows with varying column lengths.
         * @param policy Possible values are IGNORE_ROW, THROW, KEEP
         */
        CSVFormat& variable_columns(VariableColumnPolicy policy);

        /**
         * @brief Sets variable column policy to either KEEP or IGNORE_ROW based on bool.
         * @param policy If true, keep variable-length columns. If false, ignore them.
         */
        CSVFormat& variable_columns(bool policy);

        /**
         * @brief Internal method for indicating that the CSV's delimiter should be guessed.
         * @return True if multiple possible delimiters are set.
         */
        bool guess_delim();

        /**
         * @brief Retrieve the single delimiter, or first possible delimiter if multiple.
         * @throws std::runtime_error if multiple delimiters are set.
         */
        char get_delim() const;

        /** @name Internals for CSVReader */
        ///@{
        bool is_quoting_enabled() const;
        char get_quote_char() const;
        int get_header() const;
        std::vector<char> get_possible_delims() const;
        std::vector<char> get_trim_chars() const;
        VariableColumnPolicy get_variable_column_policy() const;
        ///@}

        /**
         * @brief A handy static CSVFormat which can guess CSV delimiter.
         */
        static CSVFormat guess_csv();

    private:
        void assert_no_char_overlap();
        std::vector<char> possible_delimiters = { ',' };
        std::vector<char> trim_chars = {};
        int header = 0;
        bool no_quote = false;
        char quote_char = '"';
        std::vector<std::string> col_names = {};
        VariableColumnPolicy variable_column_policy = VariableColumnPolicy::IGNORE_ROW;
    };

    /** @class CSVReader
     *  @brief Main interface for reading CSV files or in-memory data.
     *
     *  The CSVReader supports:
     *   - Iterating over CSV rows via `begin()` / `end()`
     *   - Reading rows one-by-one with `read_row()`
     *   - Retrieving CSV metadata like column names, or the total # of rows
     *
     *  Example usage:
     *  \code
     *  csv::CSVReader reader("my_file.csv");
     *  for (auto& row : reader) {
     *      std::string colA = row["Column A"].get<std::string>();
     *  }
     *  \endcode
     */
    class CSVReader {
    public:
        /**
         * @class iterator
         * @brief An input iterator over the CSV's rows.
         *
         * Example usage:
         * \code
         * csv::CSVReader reader("my_data.csv");
         * for (auto it = reader.begin(); it != reader.end(); ++it) {
         *     CSVRow row = *it;
         * }
         * \endcode
         */
        class iterator {
        public:
            using value_type = CSVRow;
            using difference_type = std::ptrdiff_t;
            using pointer = CSVRow*;
            using reference = CSVRow&;
            using iterator_category = std::input_iterator_tag;

            /**
             * @brief Dereference operator to obtain the current row.
             */
            reference operator*();
            /**
             * @brief Pointer access to the current row.
             */
            pointer operator->();

            /**
             * @brief Pre-increment operator.
             */
            iterator& operator++();
            /**
             * @brief Post-increment operator.
             */
            iterator operator++(int);
            /**
             * @brief Pre-decrement operator.
             */
            iterator& operator--();

            /**
             * @brief Compare two iterators for equality.
             */
            constexpr bool operator==(const iterator& other) const noexcept;
            /**
             * @brief Compare two iterators for inequality.
             */
            constexpr bool operator!=(const iterator& other) const noexcept;

        private:
            friend CSVReader;
            iterator() = default;
            iterator(CSVReader* reader);
            iterator(CSVReader* reader, CSVRow&& row);

            CSVReader * daddy = nullptr;
            CSVRow row;
            size_t i = 0;
        };

        /**
         * @brief Construct a CSVReader for a given file on disk.
         * @param filename Path to the CSV file
         * @param format CSVFormat configuration to interpret the file
         *
         * If the format has multiple possible delimiters,
         * the parser will guess which delimiter is actually used by the file.
         * Otherwise, it uses the single specified delimiter.
         */
        CSVReader(csv::string_view filename, CSVFormat format = CSVFormat::guess_csv());

        /**
         * @brief Construct a CSVReader from any std::istream source (e.g. std::stringstream).
         * @tparam TStream Must be derived from std::istream.
         * @param source The input stream
         * @param format CSVFormat configuration
         */
        template<typename TStream,
            csv::enable_if_t<std::is_base_of<std::istream, TStream>::value, int> = 0>
        CSVReader(TStream& source, CSVFormat format = CSVFormat());

        // No copy constructor
        CSVReader(const CSVReader&) = delete;
        // Move constructor
        CSVReader(CSVReader&&) = default;

        // No copy assignment
        CSVReader& operator=(const CSVReader&) = delete;
        // Move assignment
        CSVReader& operator=(CSVReader&&) = default;

        /**
         * @brief Destructor. Joins any background parsing threads.
         */
        ~CSVReader();

        /**
         * @brief Attempt to read the next row from the CSV. Returns false if no more rows exist.
         * @param row A CSVRow object which will be populated.
         * @return true if a row was successfully read, false if EOF.
         */
        bool read_row(CSVRow &row);

        /**
         * @name Iterators
         * @brief Access the CSV file row-by-row using standard input iterator semantics.
         *
         * Example:
         * \code
         * csv::CSVReader reader("my_file.csv");
         * for (auto& row : reader) {
         *     // do something
         * }
         * \endcode
         */
        ///@{
        iterator begin();
        constexpr iterator end() const noexcept;
        ///@}

        /**
         * @brief Checks if we have reached end of file.
         */
        bool eof() const noexcept;

        /**
         * @name CSV Metadata
         */
        ///@{
        /**
         * @brief Returns the CSV format that the reader is using (delim, quote, etc.).
         */
        CSVFormat get_format() const;

        /**
         * @brief Retrieve the column names of this CSV.
         */
        std::vector<std::string> get_col_names() const;

        /**
         * @brief Get the index of a specific column name, or csv::CSV_NOT_FOUND if it does not exist.
         */
        int index_of(csv::string_view col_name) const;

        /**
         * @brief Returns true if the CSV is empty (no rows beyond possible header row).
         */
        constexpr bool empty() const noexcept;

        /**
         * @brief Returns how many rows (excluding header) have been parsed so far.
         */
        constexpr size_t n_rows() const noexcept;

        /**
         * @brief Checks whether the file had a UTF-8 BOM at the start.
         */
        bool utf8_bom() const noexcept;
        ///@}

    protected:
        // For advanced usage only
        void set_col_names(const std::vector<std::string>&);
        bool read_csv(size_t bytes);
        void trim_header();
        void initial_read();

        CSVFormat _format;
        std::shared_ptr<internals::ColNames> col_names;
        std::unique_ptr<internals::IBasicCSVParser> parser;
        std::unique_ptr<internals::ThreadSafeDeque<CSVRow>> records;
        std::thread read_csv_worker;

        size_t n_cols = 0;
        size_t _n_rows = 0;
        bool header_trimmed = false;

    private:
        bool eof_flag = false;
    };

    /**
     * @class CSVStat
     * @brief Collects column-based statistics from a CSV file or stream.
     *
     * The data includes:
     *  - Mean
     *  - Variance
     *  - Minimum, Maximum
     *  - Frequency counts
     *  - Type counts (CSV_STRING, CSV_INT, CSV_DOUBLE, etc.)
     *
     * Usage example:
     * \code
     * csv::CSVStat stats("my_data.csv");
     * auto means = stats.get_mean();
     * auto mins  = stats.get_mins();
     * \endcode
     */
    class CSVStat {
    public:
        using FreqCount = std::unordered_map<std::string, size_t>;
        using TypeCount = std::unordered_map<DataType, size_t>;

        /**
         * @brief Return the calculated means of all numeric columns.
         */
        std::vector<long double> get_mean() const;

        /**
         * @brief Return the variances for all numeric columns.
         */
        std::vector<long double> get_variance() const;

        /**
         * @brief Return the minimum values for all numeric columns.
         */
        std::vector<long double> get_mins() const;

        /**
         * @brief Return the maximum values for all numeric columns.
         */
        std::vector<long double> get_maxes() const;

        /**
         * @brief Return the frequency counts of each column.
         * @return One FreqCount map per column.
         */
        std::vector<FreqCount> get_counts() const;

        /**
         * @brief Return the data type counts of each column.
         * @return One TypeCount map per column.
         */
        std::vector<TypeCount> get_dtypes() const;

        /**
         * @brief Get column names from the underlying CSVReader.
         */
        std::vector<std::string> get_col_names() const;

        /**
         * @brief Construct a CSVStat over a file on disk.
         * @param filename Path to CSV file
         * @param format CSVFormat specifying how to parse
         */
        CSVStat(csv::string_view filename, CSVFormat format = CSVFormat::guess_csv());

        /**
         * @brief Construct a CSVStat from a std::stringstream-based CSV.
         * @param source Input stream
         * @param format CSVFormat specifying how to parse
         */
        CSVStat(std::stringstream& source, CSVFormat format = CSVFormat());

    private:
        void variance(const long double&, const size_t&);
        void count(CSVField&, const size_t&);
        void min_max(const long double&, const size_t&);
        void dtype(CSVField&, const size_t&);
        void calc();
        void calc_chunk();
        void calc_worker(const size_t&);

        CSVReader reader;
        std::deque<CSVRow> records;

        std::vector<long double> rolling_means;
        std::vector<long double> rolling_vars;
        std::vector<long double> mins;
        std::vector<long double> maxes;
        std::vector<FreqCount> counts;
        std::vector<TypeCount> dtypes;
        std::vector<long double> n;
    };

    /**
     * @brief Return a CSVReader by parsing a string literal.
     *
     * Example usage:
     * \code
     * using namespace csv::literals;
     * auto reader = "a,b\n1,2\n"_csv;
     * for (auto& row : reader) {
     *     // ...
     * }
     * \endcode
     *
     * @param in A pointer to the CSV data
     * @param n  The length of the string literal
     * @return A CSVReader for the data
     */
    CSVReader operator ""_csv(const char*, size_t);

    /**
     * @brief Return a CSVReader by parsing a string literal with no header assumed.
     *
     * \code
     * using namespace csv::literals;
     * auto reader = "val1,val2\nval3,val4\n"_csv_no_header;
     * \endcode
     */
    CSVReader operator ""_csv_no_header(const char*, size_t);

    /**
     * @brief Parse an in-memory CSV string using the given format.
     * @param in The CSV data
     * @param format The CSV parsing format
     * @return A CSVReader for iterating over the data
     */
    CSVReader parse(csv::string_view in, CSVFormat format = CSVFormat());

    /**
     * @brief Parse an in-memory CSV string with no header row.
     * @param in The CSV data
     * @return A CSVReader
     */
    CSVReader parse_no_header(csv::string_view in);

    /**
     * @brief An integer-based data type inference for each column in a CSV.
     *
     * This function scans the entire file once and classifies each column
     * based on the largest type needed (CSV_INT8, CSV_INT16, CSV_INT32, CSV_INT64, CSV_DOUBLE)
     * or CSV_STRING if non-numeric data is found.
     *
     * @param filename The CSV file path
     * @return A mapping of column name to data type
     */
    std::unordered_map<std::string, DataType> csv_data_types(const std::string&);

    /**
     * @struct CSVFileInfo
     * @brief Basic information about a CSV file.
     */
    struct CSVFileInfo {
        std::string filename;          /**< Filename */
        std::vector<std::string> col_names; /**< List of column names */
        char delim;                    /**< Delimiting character */
        size_t n_rows;                 /**< Number of data rows (excluding header) */
        size_t n_cols;                 /**< Number of columns in the CSV */
    };

    /**
     * @brief Retrieve basic information (column names, delim, etc.) about a CSV file.
     * @param filename Path to the CSV file
     * @return A CSVFileInfo object containing metadata.
     */
    CSVFileInfo get_file_info(const std::string& filename);

    /**
     * @brief Return the position of a given column within a CSV file. Returns CSV_NOT_FOUND if not present.
     * @param filename Path to the CSV file
     * @param col_name Name of the column to locate
     * @param format CSVFormat describing how the file is structured
     */
    int get_col_pos(csv::string_view filename, csv::string_view col_name,
        const CSVFormat& format = CSVFormat::guess_csv());

    /** 
     * @brief Controls how many decimal places are used when writing floating point values.
     * @param precision Number of digits after the decimal point
     * @note Affects the CSV/TSV writers globally.
     */
    inline static void set_decimal_places(int precision);

    /**
     * @class DelimWriter
     * @brief A generic writer for producing files with a given delimiter and quote character.
     *
     * Typically, you use one of the aliases below:
     *  - csv::CSVWriter<std::ofstream> (comma separated)
     *  - csv::TSVWriter<std::ofstream> (tab separated)
     */
    template<class OutputStream, char Delim, char Quote, bool Flush>
    class DelimWriter {
    public:
        /**
         * @brief Constructs a DelimWriter to write data to a given output stream.
         * @param _out The output stream (e.g. std::ofstream)
         * @param _quote_minimal If true, only quote fields when necessary
         */
        DelimWriter(OutputStream& _out, bool _quote_minimal = true);

        /**
         * @brief Constructs a DelimWriter to write data to a file by name.
         * @param filename File path to be written to
         */
        DelimWriter(const std::string& filename);

        /**
         * @brief Destructor flushes any remaining data to the output stream.
         */
        ~DelimWriter();

        /**
         * @brief Write a std::array<T, Size> as a row in the file.
         * @tparam T Type of elements
         * @tparam Size Size of the array
         */
        template<typename T, size_t Size>
        DelimWriter& operator<<(const std::array<T, Size>& record);

        /**
         * @brief Write a std::tuple of elements as a row in the file.
         *
         * The entire tuple is expanded into a comma/tab separated line.
         */
        template<typename... T>
        DelimWriter& operator<<(const std::tuple<T...>& record);

        /**
         * @brief Write a container (e.g. std::vector) of elements as a row in the file.
         *
         * Each element becomes a separate column.
         */
        template<typename T, typename Alloc, template <typename, typename> class Container,
            csv::enable_if_t<std::is_class<Alloc>::value, int> = 0>
        DelimWriter& operator<<(const Container<T, Alloc>& record);

        /**
         * @brief Manually flush all data to the underlying stream.
         */
        void flush();

    private:
        // Internal helpers
        template<typename T, csv::enable_if_t<!std::is_convertible<T, std::string>::value
            && !std::is_convertible<T, csv::string_view>::value, int> = 0>
        std::string csv_escape(T in);

        template<typename T, csv::enable_if_t<std::is_convertible<T, std::string>::value
            || std::is_convertible<T, csv::string_view>::value, int> = 0>
        std::string csv_escape(T in);

        std::string _csv_escape(csv::string_view in);
        template<size_t Index = 0, typename... T>
        typename std::enable_if<Index < sizeof...(T), void>::type write_tuple(const std::tuple<T...>& record);

        template<size_t Index = 0, typename... T>
        typename std::enable_if<Index == sizeof...(T), void>::type write_tuple(const std::tuple<T...>& record);

        void end_out();

        OutputStream& out;
        bool quote_minimal;
    };

    /**
     * @class CSVWriter
     * @brief An alias for DelimWriter using comma (',') as a delimiter and double-quote ('"') as quote char.
     * @tparam OutputStream Type of underlying output stream (usually std::ofstream)
     * @tparam Flush If true, automatically flushes after each row
     */
    template<class OutputStream, bool Flush = true>
    using CSVWriter = DelimWriter<OutputStream, ',', '"', Flush>;

    /**
     * @class TSVWriter
     * @brief An alias for DelimWriter using tab ('\t') as a delimiter and double-quote ('"') as quote char.
     * @tparam OutputStream Type of underlying output stream
     * @tparam Flush If true, automatically flushes after each row
     */
    template<class OutputStream, bool Flush = true>
    using TSVWriter = DelimWriter<OutputStream, '\t', '"', Flush>;

    /**
     * @brief Helper function to create a CSVWriter over an existing output stream, auto-flushing each row.
     * @param out The output stream
     * @param quote_minimal If true, only quote when necessary
     */
    template<class OutputStream>
    inline CSVWriter<OutputStream> make_csv_writer(OutputStream& out, bool quote_minimal=true);

    /**
     * @brief Helper function to create a CSVWriter that does not auto-flush.
     * @param out The output stream
     * @param quote_minimal If true, only quote when necessary
     */
    template<class OutputStream>
    inline CSVWriter<OutputStream, false> make_csv_writer_buffered(OutputStream& out, bool quote_minimal=true);

    /**
     * @brief Helper function to create a TSVWriter over an existing output stream, auto-flushing each row.
     * @param out The output stream
     * @param quote_minimal If true, only quote when necessary
     */
    template<class OutputStream>
    inline TSVWriter<OutputStream> make_tsv_writer(OutputStream& out, bool quote_minimal=true);

    /**
     * @brief Helper function to create a TSVWriter that does not auto-flush.
     * @param out The output stream
     * @param quote_minimal If true, only quote when necessary
     */
    template<class OutputStream>
    inline TSVWriter<OutputStream, false> make_tsv_writer_buffered(OutputStream& out, bool quote_minimal=true);

} // end namespace csv

#include "csv.cpp"

#endif
