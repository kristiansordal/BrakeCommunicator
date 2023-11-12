/******************************************************************************
 * Copyright (c) 2011, Duane Merrill.  All rights reserved.
 * Copyright (c) 2011-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the NVIDIA CORPORATION nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NVIDIA CORPORATION BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

/******************************************************************************
 * Matrix data structures and parsing logic
 ******************************************************************************/

#pragma once

#include <cmath>
#include <cstring>

#include <algorithm>
#include <bits/stdc++.h>
#include <fstream>
#include <iostream>
#include <iterator>
#include <list>
#include <queue>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef CUB_MKL
#include <numa.h>
#ifdef EVAL_MKL
#include <mkl.h>
#endif
#endif

#include <machine/endian.h>
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define BYTE_ORDER_STR "LE"
#elif __BYTE_ORDER == __BIG_ENDIAN
#define BYTE_ORDER_STR "BE"
#elif __BYTE_ORDER == __PDP_ENDIAN
#define BYTE_ORDER_STR "PE"
#else
#error "no byte order"
#endif

using namespace std;

/******************************************************************************
 * Graph stats
 ******************************************************************************/

struct GraphStats {
    int num_rows;
    int num_cols;
    int num_nonzeros;

    double pearson_r; // coefficient of variation x vs y (how linear the sparsity plot is)

    double row_length_mean;      // mean
    double row_length_std_dev;   // sample std_dev
    double row_length_variation; // coefficient of variation
    double row_length_skewness;  // skewness

    void Display(bool show_labels = true) {
        if (show_labels)
            printf("\n"
                   "\t num_rows: %d\n"
                   "\t num_cols: %d\n"
                   "\t num_nonzeros: %d\n"
                   "\t row_length_mean: %.5f\n"
                   "\t row_length_std_dev: %.5f\n"
                   "\t row_length_variation: %.5f\n"
                   "\t row_length_skewness: %.5f\n",
                   num_rows, num_cols, num_nonzeros, row_length_mean, row_length_std_dev, row_length_variation,
                   row_length_skewness);
        else
            printf("%d, "
                   "%d, "
                   "%d, "
                   "%.5f, "
                   "%.5f, "
                   "%.5f, "
                   "%.5f, ",
                   num_rows, num_cols, num_nonzeros, row_length_mean, row_length_std_dev, row_length_variation,
                   row_length_skewness);
    }
};

/******************************************************************************
 * COO matrix type
 ******************************************************************************/

/**
 * COO matrix type.  A COO matrix is just a vector of edge tuples.  Tuples are sorted
 * first by row, then by column.
 */
template <typename ValueT, typename OffsetT> struct CooMatrix {
    //---------------------------------------------------------------------
    // Type definitions and constants
    //---------------------------------------------------------------------

    // COO edge tuple
    struct CooTuple {
        OffsetT row;
        OffsetT col;
        ValueT val;

        CooTuple() {}
        CooTuple(OffsetT row, OffsetT col)
            : row(row),
              col(col) {}
        CooTuple(OffsetT row, OffsetT col, ValueT val)
            : row(row),
              col(col),
              val(val) {}
    };

    //---------------------------------------------------------------------
    // Data members
    //---------------------------------------------------------------------

    // Fields
    OffsetT num_rows;
    OffsetT num_cols;
    OffsetT num_nonzeros;
    CooTuple *coo_tuples;

    //---------------------------------------------------------------------
    // Methods
    //---------------------------------------------------------------------

    // Constructor
    CooMatrix()
        : num_rows(0),
          num_cols(0),
          num_nonzeros(0),
          coo_tuples(NULL) {}

    /**
     * Clear
     */
    void Clear() {
        if (coo_tuples)
            delete[] coo_tuples;
        coo_tuples = NULL;
    }

    // Destructor
    ~CooMatrix() { Clear(); }

    // Display matrix to stdout
    void Display() {
        cout << "COO Matrix (" << num_rows << " rows, " << num_cols << " columns, " << num_nonzeros << " non-zeros):\n";
        cout << "Ordinal, Row, Column, Value\n";
        for (OffsetT i = 0; i < num_nonzeros; i++) {
            cout << '\t' << i << ',' << coo_tuples[i].row << ',' << coo_tuples[i].col << ',' << coo_tuples[i].val
                 << "\n";
        }
    }

    /**
     * Builds a COO sparse from a relabeled CSR matrix.
     */
    template <typename CsrMatrixT> void InitCsrRelabel(CsrMatrixT &csr_matrix, OffsetT *relabel_indices) {
        if (coo_tuples) {
            fprintf(stderr, "Matrix already constructed\n");
            exit(1);
        }

        num_rows = csr_matrix.num_rows;
        num_cols = csr_matrix.num_cols;
        num_nonzeros = csr_matrix.num_nonzeros;
        coo_tuples = new CooTuple[num_nonzeros];

        for (OffsetT row = 0; row < num_rows; ++row) {
            for (OffsetT nonzero = csr_matrix.row_offsets[row]; nonzero < csr_matrix.row_offsets[row + 1]; ++nonzero) {
                coo_tuples[nonzero].row = relabel_indices[row];
                coo_tuples[nonzero].col = relabel_indices[csr_matrix.column_indices[nonzero]];
                coo_tuples[nonzero].val = csr_matrix.values[nonzero];
            }
        }
    }

    /**
     * Builds a MARKET COO sparse from the given file.
     */
    void InitMarket(const string &market_filename, ValueT default_value = 1.0, bool verbose = false) {
        if (verbose) {
            printf("Reading... ");
            fflush(stdout);
        }

        if (coo_tuples) {
            fprintf(stderr, "Matrix already constructed\n");
            exit(1);
        }

        std::ifstream ifs;
        ifs.open(market_filename.c_str(), std::ifstream::in);
        if (!ifs.good()) {
            fprintf(stderr, "Error opening file\n");
            exit(1);
        }

        bool array = false;
        bool symmetric = false;
        bool skew = false;
        OffsetT current_nz = -1;
        char line[1024];

        if (verbose) {
            printf("Parsing... ");
            fflush(stdout);
        }

        while (true) {
            ifs.getline(line, 1024);
            if (!ifs.good()) {
                // Done
                break;
            }

            if (line[0] == '%') {
                // Comment
                if (line[1] == '%') {
                    // Banner
                    symmetric = (strstr(line, "symmetric") != NULL);
                    skew = (strstr(line, "skew") != NULL);
                    array = (strstr(line, "array") != NULL);

                    if (verbose) {
                        printf("(symmetric: %d, skew: %d, array: %d) ", symmetric, skew, array);
                        fflush(stdout);
                    }
                }
            } else if (current_nz == -1) {
                // Problem description
                OffsetT nparsed = 0;
                if (sizeof(OffsetT) <= sizeof(int)) {
                    nparsed = sscanf(line, "%d %d %d", &num_rows, &num_cols, &num_nonzeros);
                } else {
                    nparsed = sscanf(line, "%ld %ld %ld", &num_rows, &num_cols, &num_nonzeros);
                }
                if ((!array) && (nparsed == 3)) {
                    if (symmetric)
                        num_nonzeros *= 2;

                    // Allocate coo matrix
                    coo_tuples = new CooTuple[num_nonzeros];
                    current_nz = 0;

                } else if (array && (nparsed == 2)) {
                    // Allocate coo matrix
                    num_nonzeros = num_rows * num_cols;
                    coo_tuples = new CooTuple[num_nonzeros];
                    current_nz = 0;
                } else {
                    fprintf(stderr, "Error parsing MARKET matrix: invalid problem description: %s\n", line);
                    exit(1);
                }

            } else {
                // Edge
                if (current_nz >= num_nonzeros) {
                    fprintf(stderr, "Error parsing MARKET matrix: encountered more than %d num_nonzeros\n",
                            num_nonzeros);
                    exit(1);
                }

                OffsetT row, col;
                double val;

                if (array) {
                    if (sscanf(line, "%lf", &val) != 1) {
                        fprintf(stderr, "Error parsing MARKET matrix: badly formed current_nz: '%s' at edge %d\n", line,
                                current_nz);
                        exit(1);
                    }
                    col = (current_nz / num_rows);
                    row = (current_nz - (num_rows * col));

                    coo_tuples[current_nz] = CooTuple(row, col, val); // Convert indices to zero-based
                } else {
                    // Parse nonzero (note: using strtol and strtod is 2x faster than sscanf or istream parsing)
                    char *l = line;
                    char *t = NULL;

                    // parse row
                    row = strtol(l, &t, 0);
                    if (t == l) {
                        fprintf(stderr, "Error parsing MARKET matrix: badly formed row at edge %d\n", current_nz);
                        exit(1);
                    }
                    l = t;

                    // parse col
                    col = strtol(l, &t, 0);
                    if (t == l) {
                        fprintf(stderr, "Error parsing MARKET matrix: badly formed col at edge %d\n", current_nz);
                        exit(1);
                    }
                    l = t;

                    // parse val
                    val = strtod(l, &t);
                    if (t == l) {
                        val = default_value;
                    }

                    coo_tuples[current_nz] = CooTuple(row - 1, col - 1, val); // Convert indices to zero-based
                }

                current_nz++;

                if (symmetric && (row != col)) {
                    coo_tuples[current_nz].row = coo_tuples[current_nz - 1].col;
                    coo_tuples[current_nz].col = coo_tuples[current_nz - 1].row;
                    coo_tuples[current_nz].val = coo_tuples[current_nz - 1].val * (skew ? -1 : 1);
                    current_nz++;
                }
            }
        }

        // Adjust nonzero count (nonzeros along the diagonal aren't reversed)
        num_nonzeros = current_nz;

        if (verbose) {
            printf("done. ");
            fflush(stdout);
        }

        ifs.close();
    }

    /**
     * Builds a dense matrix
     */
    int InitDense(OffsetT num_rows, OffsetT num_cols, ValueT default_value = 1.0, bool verbose = false) {
        if (coo_tuples) {
            fprintf(stderr, "Matrix already constructed\n");
            exit(1);
        }

        this->num_rows = num_rows;
        this->num_cols = num_cols;

        num_nonzeros = num_rows * num_cols;
        coo_tuples = new CooTuple[num_nonzeros];

        for (OffsetT row = 0; row < num_rows; ++row) {
            for (OffsetT col = 0; col < num_cols; ++col) {
                coo_tuples[(row * num_cols) + col] = CooTuple(row, col, default_value);
            }
        }

        return 0;
    }

    /**
     * Builds a wheel COO sparse matrix having spokes spokes.
     */
    int InitWheel(OffsetT spokes, ValueT default_value = 1.0, bool verbose = false) {
        if (coo_tuples) {
            fprintf(stderr, "Matrix already constructed\n");
            exit(1);
        }

        num_rows = spokes + 1;
        num_cols = num_rows;
        num_nonzeros = spokes * 2;
        coo_tuples = new CooTuple[num_nonzeros];

        // Add spoke num_nonzeros
        OffsetT current_nz = 0;
        for (OffsetT i = 0; i < spokes; i++) {
            coo_tuples[current_nz] = CooTuple(0, i + 1, default_value);
            current_nz++;
        }

        // Add rim
        for (OffsetT i = 0; i < spokes; i++) {
            OffsetT dest = (i + 1) % spokes;
            coo_tuples[current_nz] = CooTuple(i + 1, dest + 1, default_value);
            current_nz++;
        }

        return 0;
    }

    /**
     * Builds a square 2D grid CSR matrix.  Interior num_vertices have degree 5 when including
     * a self-loop.
     *
     * Returns 0 on success, 1 on failure.
     */
    int InitGrid2d(OffsetT width, bool self_loop, ValueT default_value = 1.0) {
        if (coo_tuples) {
            fprintf(stderr, "Matrix already constructed\n");
            exit(1);
        }

        OffsetT interior_nodes = (width - 2) * (width - 2);
        OffsetT edge_nodes = (width - 2) * 4;
        OffsetT corner_nodes = 4;
        num_rows = width * width;
        num_cols = num_rows;
        num_nonzeros = (interior_nodes * 4) + (edge_nodes * 3) + (corner_nodes * 2);

        if (self_loop)
            num_nonzeros += num_rows;

        coo_tuples = new CooTuple[num_nonzeros];
        OffsetT current_nz = 0;

        for (OffsetT j = 0; j < width; j++) {
            for (OffsetT k = 0; k < width; k++) {
                OffsetT me = (j * width) + k;

                // West
                OffsetT neighbor = (j * width) + (k - 1);
                if (k - 1 >= 0) {
                    coo_tuples[current_nz] = CooTuple(me, neighbor, default_value);
                    current_nz++;
                }

                // East
                neighbor = (j * width) + (k + 1);
                if (k + 1 < width) {
                    coo_tuples[current_nz] = CooTuple(me, neighbor, default_value);
                    current_nz++;
                }

                // North
                neighbor = ((j - 1) * width) + k;
                if (j - 1 >= 0) {
                    coo_tuples[current_nz] = CooTuple(me, neighbor, default_value);
                    current_nz++;
                }

                // South
                neighbor = ((j + 1) * width) + k;
                if (j + 1 < width) {
                    coo_tuples[current_nz] = CooTuple(me, neighbor, default_value);
                    current_nz++;
                }

                if (self_loop) {
                    neighbor = me;
                    coo_tuples[current_nz] = CooTuple(me, neighbor, default_value);
                    current_nz++;
                }
            }
        }

        return 0;
    }

    /**
     * Builds a square 3D grid COO sparse matrix.  Interior num_vertices have degree 7 when including
     * a self-loop.  Values are unintialized, coo_tuples are sorted.
     */
    int InitGrid3d(OffsetT width, bool self_loop, ValueT default_value = 1.0) {
        if (coo_tuples) {
            fprintf(stderr, "Matrix already constructed\n");
            return -1;
        }

        OffsetT interior_nodes = (width - 2) * (width - 2) * (width - 2);
        OffsetT face_nodes = (width - 2) * (width - 2) * 6;
        OffsetT edge_nodes = (width - 2) * 12;
        OffsetT corner_nodes = 8;
        num_cols = width * width * width;
        num_rows = num_cols;
        num_nonzeros = (interior_nodes * 6) + (face_nodes * 5) + (edge_nodes * 4) + (corner_nodes * 3);

        if (self_loop)
            num_nonzeros += num_rows;

        coo_tuples = new CooTuple[num_nonzeros];
        OffsetT current_nz = 0;

        for (OffsetT i = 0; i < width; i++) {
            for (OffsetT j = 0; j < width; j++) {
                for (OffsetT k = 0; k < width; k++) {

                    OffsetT me = (i * width * width) + (j * width) + k;

                    // Up
                    OffsetT neighbor = (i * width * width) + (j * width) + (k - 1);
                    if (k - 1 >= 0) {
                        coo_tuples[current_nz] = CooTuple(me, neighbor, default_value);
                        current_nz++;
                    }

                    // Down
                    neighbor = (i * width * width) + (j * width) + (k + 1);
                    if (k + 1 < width) {
                        coo_tuples[current_nz] = CooTuple(me, neighbor, default_value);
                        current_nz++;
                    }

                    // West
                    neighbor = (i * width * width) + ((j - 1) * width) + k;
                    if (j - 1 >= 0) {
                        coo_tuples[current_nz] = CooTuple(me, neighbor, default_value);
                        current_nz++;
                    }

                    // East
                    neighbor = (i * width * width) + ((j + 1) * width) + k;
                    if (j + 1 < width) {
                        coo_tuples[current_nz] = CooTuple(me, neighbor, default_value);
                        current_nz++;
                    }

                    // North
                    neighbor = ((i - 1) * width * width) + (j * width) + k;
                    if (i - 1 >= 0) {
                        coo_tuples[current_nz] = CooTuple(me, neighbor, default_value);
                        current_nz++;
                    }

                    // South
                    neighbor = ((i + 1) * width * width) + (j * width) + k;
                    if (i + 1 < width) {
                        coo_tuples[current_nz] = CooTuple(me, neighbor, default_value);
                        current_nz++;
                    }

                    if (self_loop) {
                        neighbor = me;
                        coo_tuples[current_nz] = CooTuple(me, neighbor, default_value);
                        current_nz++;
                    }
                }
            }
        }

        return 0;
    }
};

/******************************************************************************
 * CSR matrix type
 ******************************************************************************/

/**
 * CSR sparse format matrix
 */
template <typename ValueT, typename OffsetT> struct CsrMatrix {
    // Sort by rows, then columns
    struct CooComparator {
        template <typename CooTuple> bool operator()(const CooTuple &a, const CooTuple &b) const {
            return ((a.row < b.row) || ((a.row == b.row) && (a.col < b.col)));
        }
    };

    OffsetT num_rows;
    OffsetT num_cols;
    OffsetT num_nonzeros;
    OffsetT *row_offsets;
    OffsetT *column_indices;
    ValueT *values;
    bool custom_alloc;

    // Whether to use NUMA malloc to always put storage on the same sockets (for perf repeatability)
    bool IsNumaMalloc() {
#ifdef CUB_MKL
        return (numa_available() >= 0);
#else
        return false;
#endif
    }

    string GenCsrFileName(string mtx_file_name) {
        string csr_file_name = mtx_file_name;
        size_t last_dot_index = csr_file_name.find_last_of(".");
        if (last_dot_index != string::npos) {
            csr_file_name = csr_file_name.substr(0, last_dot_index);
        }
        // csr_file_name += string() + "." + typeid(ValueT).name() + "_" + typeid(OffsetT).name() + "_" + BYTE_ORDER_STR
        // + ".csr";
        csr_file_name += string() + "." + std::to_string(sizeof(ValueT)) + "_" + std::to_string(sizeof(OffsetT)) + "_" +
                         BYTE_ORDER_STR + ".csr";
        return csr_file_name;
    }

    /**
     * Allocator
     */
    void Alloc(bool verbose = false) {
        // printf("Alloc()\n"); fflush(stdout);
        // printf("  num_rows=%d\n", num_rows); fflush(stdout);
        // printf("  num_cols=%d\n", num_cols); fflush(stdout);
        // printf("  num_nonzeros=%d\n", num_nonzeros); fflush(stdout);
        // printf("  sizeof(OffsetT)=%d\n", sizeof(OffsetT)); fflush(stdout);
        // printf("  sizeof(ValueT)=%d\n", sizeof(ValueT)); fflush(stdout);
#ifdef CUB_MKL

        if (IsNumaMalloc()) {
            // printf("  Using NumaMalloc\n");
            numa_set_strict(1);

            row_offsets = (OffsetT *)numa_alloc_onnode(sizeof(OffsetT) * (num_rows + 1), 0);
            column_indices = (OffsetT *)numa_alloc_onnode(sizeof(OffsetT) * num_nonzeros, 0);

            if (numa_num_task_nodes() > 1)
                values = (ValueT *)numa_alloc_onnode(sizeof(ValueT) * num_nonzeros,
                                                     1); // put on different socket than column_indices
            else
                values = (ValueT *)numa_alloc_onnode(sizeof(ValueT) * num_nonzeros, 0);
        } else {
#ifdef EVAL_MKL
            // printf("  Using mkl_malloc\n");
            values = (ValueT *)mkl_malloc(sizeof(ValueT) * num_nonzeros, 4096);
            row_offsets = (OffsetT *)mkl_malloc(sizeof(OffsetT) * (num_rows + 1), 4096);
            column_indices = (OffsetT *)mkl_malloc(sizeof(OffsetT) * num_nonzeros, 4096);
#else
            // printf("  Using malloc\n");
            values = (ValueT *)malloc(sizeof(ValueT) * num_nonzeros);
            row_offsets = (OffsetT *)malloc(sizeof(OffsetT) * (num_rows + 1));
            column_indices = (OffsetT *)malloc(sizeof(OffsetT) * num_nonzeros);
#endif
        }

#else
        // printf("  Using new\n");
        row_offsets = new OffsetT[num_rows + 1];
        column_indices = new OffsetT[num_nonzeros];
        values = new ValueT[num_nonzeros];
#endif
    }

    /**
     * Initializer
     */
    void Init(CooMatrix<ValueT, OffsetT> &coo_matrix, bool verbose = false) {
        num_rows = coo_matrix.num_rows;
        num_cols = coo_matrix.num_cols;
        num_nonzeros = coo_matrix.num_nonzeros;

        // Sort by rows, then columns
        if (verbose)
            printf("Ordering...");
        fflush(stdout);
        std::stable_sort(coo_matrix.coo_tuples, coo_matrix.coo_tuples + num_nonzeros, CooComparator());
        if (verbose)
            printf("done.");
        fflush(stdout);

        Alloc();

        OffsetT prev_row = -1;
        for (OffsetT current_nz = 0; current_nz < num_nonzeros; current_nz++) {
            OffsetT current_row = coo_matrix.coo_tuples[current_nz].row;

            // Fill in rows up to and including the current row
            for (OffsetT row = prev_row + 1; row <= current_row; row++) {
                row_offsets[row] = current_nz;
            }
            prev_row = current_row;

            column_indices[current_nz] = coo_matrix.coo_tuples[current_nz].col;
            values[current_nz] = coo_matrix.coo_tuples[current_nz].val;
        }

        // Fill out any trailing edgeless vertices (and the end-of-list element)
        for (OffsetT row = prev_row + 1; row <= num_rows; row++) {
            row_offsets[row] = num_nonzeros;
        }
    }

    /**
     * Clear
     */
    void Clear() {
        if (!custom_alloc) {
#ifdef CUB_MKL
            if (IsNumaMalloc()) {
                numa_free(row_offsets, sizeof(OffsetT) * (num_rows + 1));
                numa_free(values, sizeof(ValueT) * num_nonzeros);
                numa_free(column_indices, sizeof(OffsetT) * num_nonzeros);
            } else {
#ifdef EVAL_MKL
                if (row_offsets)
                    mkl_free(row_offsets);
                if (column_indices)
                    mkl_free(column_indices);
                if (values)
                    mkl_free(values);
#else
                if (row_offsets)
                    free(row_offsets);
                if (column_indices)
                    free(column_indices);
                if (values)
                    free(values);
#endif
            }

#else
            if (row_offsets)
                delete[] row_offsets;
            if (column_indices)
                delete[] column_indices;
            if (values)
                delete[] values;
#endif

            row_offsets = NULL;
            column_indices = NULL;
            values = NULL;
        }
    }

    /**
     * Assign
     */
    void Assign(OffsetT _num_rows, OffsetT _num_cols, OffsetT _num_nonzeros, OffsetT *_row_offsets,
                OffsetT *_column_indices, ValueT *_values) {
        custom_alloc = true;
        num_rows = _num_rows;
        num_cols = _num_cols;
        num_nonzeros = _num_nonzeros;
        row_offsets = _row_offsets;
        column_indices = _column_indices;
        values = _values;
    }

    /**
     * Init Shuffle Rows
     */
    void InitShuffleRows(CsrMatrix<ValueT, OffsetT> &csr_matrix, int seed = 0) {
        OffsetT *rows;

        num_rows = csr_matrix.num_rows;
        num_cols = csr_matrix.num_cols;
        num_nonzeros = csr_matrix.num_nonzeros;
        row_offsets = 0;
        column_indices = 0;
        values = 0;
        custom_alloc = false;

        Alloc();

        rows = new OffsetT[num_rows];
        for (OffsetT row = 0; row < num_rows; row++) {
            rows[row] = num_rows - 1 - row;
        }
        shuffle(rows, rows + num_rows, default_random_engine(seed));

        OffsetT col_offset = 0;
        row_offsets[0] = 0;
        for (OffsetT row = 0; row < num_rows; row++) {
            OffsetT csr_matrix_row = rows[row];
            for (OffsetT csr_matrix_col_offset = csr_matrix.row_offsets[csr_matrix_row];
                 csr_matrix_col_offset < csr_matrix.row_offsets[csr_matrix_row + 1]; csr_matrix_col_offset++) {
                OffsetT col = csr_matrix.column_indices[csr_matrix_col_offset];
                ValueT val = csr_matrix.values[csr_matrix_col_offset];
                column_indices[col_offset] = col;
                values[col_offset] = val;
                col_offset++;
            }
            row_offsets[row + 1] = col_offset;
        }

        delete[] rows;
    }

    /**
     * Init Shuffle Cols
     */
    void InitShuffleCols(CsrMatrix<ValueT, OffsetT> &csr_matrix, int seed = 0) {
        OffsetT *cols;

        num_rows = csr_matrix.num_rows;
        num_cols = csr_matrix.num_cols;
        num_nonzeros = csr_matrix.num_nonzeros;
        row_offsets = 0;
        column_indices = 0;
        values = 0;
        custom_alloc = false;

        Alloc();

        cols = new OffsetT[num_cols];
        for (OffsetT col = 0; col < num_cols; col++) {
            cols[col] = num_cols - 1 - col;
        }
        // shuffle(cols, cols + num_cols, default_random_engine(seed));

        OffsetT row = 0;
        OffsetT col_offset = 0;
        row_offsets[0] = 0;
        pair<OffsetT, ValueT> *row_data = 0;
        OffsetT max_row_len = 0;
        for (OffsetT csr_matrix_row = 0; csr_matrix_row < num_rows; csr_matrix_row++) {
            OffsetT row_len = csr_matrix.row_offsets[csr_matrix_row + 1] - csr_matrix.row_offsets[csr_matrix_row];
            if (max_row_len < row_len) {
                if (row_data) {
                    delete[] row_data;
                }
                max_row_len = row_len;
                row_data = new pair<OffsetT, ValueT>[max_row_len];
            }
            for (OffsetT idx = 0; idx < row_len; idx++) {
                OffsetT col = csr_matrix.column_indices[csr_matrix.row_offsets[csr_matrix_row] + idx];
                ValueT val = csr_matrix.values[csr_matrix.row_offsets[csr_matrix_row] + idx];
                row_data[idx] = make_pair(cols[col], val);
            }
            sort(row_data, row_data + row_len);
            for (OffsetT idx = 0; idx < row_len; idx++) {
                OffsetT col = row_data[idx].first;
                ValueT val = row_data[idx].second;
                column_indices[col_offset] = col;
                values[col_offset] = val;
                col_offset++;
            }
            row_offsets[row + 1] = col_offset;
            row++;
        }

        if (row_data) {
            delete[] row_data;
        }
        delete[] cols;
    }

    /**
     * Init Shuffle Rows Cols
     */
    void InitShuffleRowsCols(CsrMatrix<ValueT, OffsetT> &csr_matrix, int seed = 0) {
        OffsetT *rows;
        OffsetT *cols;

        num_rows = csr_matrix.num_rows;
        num_cols = csr_matrix.num_cols;
        num_nonzeros = csr_matrix.num_nonzeros;
        row_offsets = 0;
        column_indices = 0;
        values = 0;
        custom_alloc = false;

        Alloc();

        rows = new OffsetT[num_rows];
        for (OffsetT row = 0; row < num_rows; row++) {
            rows[row] = num_rows - 1 - row;
        }
        // shuffle(rows, rows + num_rows, default_random_engine(seed));

        cols = new OffsetT[num_cols];
        for (OffsetT col = 0; col < num_cols; col++) {
            cols[col] = num_cols - 1 - col;
        }
        // shuffle(cols, cols + num_cols, default_random_engine(seed));

        OffsetT row = 0;
        OffsetT col_offset = 0;
        row_offsets[0] = 0;
        pair<OffsetT, ValueT> *row_data = 0;
        OffsetT max_row_len = 0;
        for (OffsetT row = 0; row < num_rows; row++) {
            OffsetT csr_matrix_row = rows[row];
            OffsetT row_len = csr_matrix.row_offsets[csr_matrix_row + 1] - csr_matrix.row_offsets[csr_matrix_row];
            if (max_row_len < row_len) {
                if (row_data) {
                    delete[] row_data;
                }
                max_row_len = row_len;
                row_data = new pair<OffsetT, ValueT>[max_row_len];
            }
            for (OffsetT idx = 0; idx < row_len; idx++) {
                OffsetT col = csr_matrix.column_indices[csr_matrix.row_offsets[csr_matrix_row] + idx];
                ValueT val = csr_matrix.values[csr_matrix.row_offsets[csr_matrix_row] + idx];
                row_data[idx] = make_pair(cols[col], val);
            }
            sort(row_data, row_data + row_len);
            for (OffsetT idx = 0; idx < row_len; idx++) {
                OffsetT col = row_data[idx].first;
                ValueT val = row_data[idx].second;
                column_indices[col_offset] = col;
                values[col_offset] = val;
                col_offset++;
            }
            row_offsets[row + 1] = col_offset;
        }

        if (row_data) {
            delete[] row_data;
        }
        delete[] cols;
        delete[] rows;
    }

    /**
     * Loader
     */
    bool Load(string mtx_file_name, bool verbose = false) {
        printf("Load(%s)\n", mtx_file_name.c_str());
        fflush(stdout);
        bool res = true;
        string csr_file_name = GenCsrFileName(mtx_file_name);
        printf("  csr_file_name=%s\n", csr_file_name.c_str());
        fflush(stdout);
        FILE *file_in = fopen(csr_file_name.c_str(), "rb");
        if (file_in) {
            if (fread(&num_rows, sizeof(num_rows), 1, file_in) != 1 || num_rows <= 0 ||
                fread(&num_cols, sizeof(num_cols), 1, file_in) != 1 || num_cols <= 0 ||
                fread(&num_nonzeros, sizeof(num_nonzeros), 1, file_in) != 1 || num_nonzeros <= 0) {
                printf("  num_rows=%d\n", num_rows);
                fflush(stdout);
                printf("  num_cols=%d\n", num_cols);
                fflush(stdout);
                printf("  num_nonzeros=%d\n", num_nonzeros);
                fflush(stdout);
                printf("  sizeof(OffsetT)=%d\n", sizeof(OffsetT));
                fflush(stdout);
                printf("  sizeof(ValueT)=%d\n", sizeof(ValueT));
                fflush(stdout);
                res = false;
                if (verbose)
                    printf("\nError reading CSR header secrion from binary file %s\n", csr_file_name.c_str());
                fflush(stdout);
            }
            Alloc();
            if (fread(row_offsets, sizeof(OffsetT), num_rows + 1, file_in) != num_rows + 1 ||
                fread(column_indices, sizeof(OffsetT), num_nonzeros, file_in) != num_nonzeros ||
                fread(values, sizeof(ValueT), num_nonzeros, file_in) != num_nonzeros) {
                res = false;
                if (verbose)
                    printf("\nError reading CSR data section from binary file %s\n", csr_file_name.c_str());
                fflush(stdout);
            }
            fclose(file_in);
            printf("  load completed\n");
            fflush(stdout);
        } else {
            res = false;
            if (verbose)
                printf("\nError opening CSR binary file %s\n", csr_file_name.c_str());
            fflush(stdout);
        }
        return res;
    }

    /**
     * Saver
     */
    bool Save(string mtx_file_name, bool verbose = false) {
        printf("Save(%s)\n", mtx_file_name.c_str());
        fflush(stdout);
        bool res = true;
        string csr_file_name = GenCsrFileName(mtx_file_name);
        printf("  csr_file_name=%s\n", csr_file_name.c_str());
        fflush(stdout);
        FILE *file_out = fopen(csr_file_name.c_str(), "wb");
        if (file_out) {
            printf("  num_rows=%d\n", num_rows);
            fflush(stdout);
            printf("  num_cols=%d\n", num_cols);
            fflush(stdout);
            printf("  num_nonzeros=%d\n", num_nonzeros);
            fflush(stdout);
            printf("  sizeof(OffsetT)=%d\n", sizeof(OffsetT));
            fflush(stdout);
            printf("  sizeof(ValueT)=%d\n", sizeof(ValueT));
            fflush(stdout);
            if (fwrite(&num_rows, sizeof(num_rows), 1, file_out) != 1 ||
                fwrite(&num_cols, sizeof(num_cols), 1, file_out) != 1 ||
                fwrite(&num_nonzeros, sizeof(num_nonzeros), 1, file_out) != 1) {
                res = false;
                if (verbose)
                    printf("\nError writing CSR header section to binary file %s\n", csr_file_name.c_str());
                fflush(stdout);
            }
            if (fwrite(row_offsets, sizeof(OffsetT), num_rows + 1, file_out) != num_rows + 1 ||
                fwrite(column_indices, sizeof(OffsetT), num_nonzeros, file_out) != num_nonzeros ||
                fwrite(values, sizeof(ValueT), num_nonzeros, file_out) != num_nonzeros) {
                res = false;
                if (verbose)
                    printf("\nError writing CSR data section to binary file %s\n", csr_file_name.c_str());
                fflush(stdout);
            }
            fclose(file_out);
            printf("  save completed\n");
            fflush(stdout);
        } else {
            res = false;
            if (verbose)
                printf("\nError creating CSR binary file %s\n", csr_file_name.c_str());
            fflush(stdout);
        }
        return res;
    }

    /**
     * Constructor
     */
    CsrMatrix() {
        custom_alloc = false;
        num_rows = 0;
        num_cols = 0;
        num_nonzeros = 0;
        row_offsets = NULL;
        column_indices = NULL;
        values = NULL;
    }

    CsrMatrix(CooMatrix<ValueT, OffsetT> &coo_matrix, bool verbose = false) {
        custom_alloc = false;
        Init(coo_matrix, verbose);
    }

    /**
     * Destructor
     */
    ~CsrMatrix() { Clear(); }

    void Features_Save(string json_file_name, string dataset_name, string group_name, string matrix_name,
                       string order_name, string file_path) {
        int new_file = 0;
        struct stat st;
        if (stat(json_file_name.c_str(), &st)) {
            new_file = 1;
        }
        if (!new_file) {
            if (st.st_size >= 6) {
                truncate(json_file_name.c_str(), st.st_size - 6);
            }
        }
        FILE *f = fopen(json_file_name.c_str(), "at");
        if (!f)
            return;
        if (new_file) {
            fprintf(f, "{\n");
            fprintf(f, "\t\"dataset\": \"%s\",\n", dataset_name.c_str());
            fprintf(f, "\t\"group\": \"%s\",\n", group_name.c_str());
            fprintf(f, "\t\"matrix\": \"%s\",\n", matrix_name.c_str());
            fprintf(f, "\t\"features\": {\n");
            fprintf(f, "\t\t\"num\": {\n");
            fprintf(f, "\t\t\t\"rows\": %d,\n", num_rows);
            fprintf(f, "\t\t\t\"cols\": %d,\n", num_cols);
            fprintf(f, "\t\t\t\"nnzs\": %d\n", num_nonzeros);
            fprintf(f, "\t\t},\n");

            //
            // Compute row-length statistics
            //
            // Sample mean
            double row_length_mean = double(num_nonzeros) / num_rows;
            double variance = 0.0;
            double row_length_skewness = 0.0;
            for (OffsetT row = 0; row < num_rows; ++row) {
                OffsetT length = row_offsets[row + 1] - row_offsets[row];
                double delta = double(length) - row_length_mean;
                variance += (delta * delta);
                row_length_skewness += (delta * delta * delta);
            }
            variance /= num_rows;
            double row_length_std_dev = sqrt(variance);
            row_length_skewness = (row_length_skewness / num_rows) / pow(row_length_std_dev, 3.0);
            double row_length_variation = row_length_std_dev / row_length_mean;
            fprintf(f, "\t\t\"row\": {\n");
            fprintf(f, "\t\t\t\"length\": {\n");
            fprintf(f, "\t\t\t\t\"mean\": %lf,\n", row_length_mean);
            fprintf(f, "\t\t\t\t\"SD\": %lf,\n", row_length_std_dev);
            fprintf(f, "\t\t\t\t\"skewness\": %lf,\n", row_length_skewness);
            fprintf(f, "\t\t\t\t\"variation\": %lf\n", row_length_variation);
            fprintf(f, "\t\t\t}\n");
            fprintf(f, "\t\t}\n");
            fprintf(f, "\t},\n");
            fprintf(f, "\t\"variants\": [\n");
        } else {
            fprintf(f, ",\n");
        }
        fprintf(f, "\t\t{\n");
        fprintf(f, "\t\t\t\"attributes\": {\n");
        fprintf(f, "\t\t\t\t\"ordering\": \"%s\"\n", order_name.c_str());
        fprintf(f, "\t\t\t},\n");
        fprintf(f, "\t\t\t\"file\": {\n");
        fprintf(f, "\t\t\t\t\"path\": \"%s\"\n", file_path.c_str());
        fprintf(f, "\t\t\t},\n");
        fprintf(f, "\t\t\t\"features\": {\n");
        //
        // Compute diag-distance statistics
        //
        OffsetT samples = 0;
        double mean = 0.0;
        double ss_tot = 0.0;
        for (OffsetT row = 0; row < num_rows; ++row) {
            OffsetT nz_idx_start = row_offsets[row];
            OffsetT nz_idx_end = row_offsets[row + 1];
            for (int nz_idx = nz_idx_start; nz_idx < nz_idx_end; ++nz_idx) {
                OffsetT col = column_indices[nz_idx];
                double x = (col > row) ? col - row : row - col;
                samples++;
                double delta = x - mean;
                mean = mean + (delta / samples);
                ss_tot += delta * (x - mean);
            }
        }
        //
        // Compute deming statistics
        //
        samples = 0;
        double mean_x = 0.0;
        double mean_y = 0.0;
        double ss_x = 0.0;
        double ss_y = 0.0;
        for (OffsetT row = 0; row < num_rows; ++row) {
            OffsetT nz_idx_start = row_offsets[row];
            OffsetT nz_idx_end = row_offsets[row + 1];
            for (int nz_idx = nz_idx_start; nz_idx < nz_idx_end; ++nz_idx) {
                OffsetT col = column_indices[nz_idx];
                samples++;
                double x = col;
                double y = row;
                double delta;
                delta = x - mean_x;
                mean_x = mean_x + (delta / samples);
                ss_x += delta * (x - mean_x);
                delta = y - mean_y;
                mean_y = mean_y + (delta / samples);
                ss_y += delta * (y - mean_y);
            }
        }
        samples = 0;
        double s_xy = 0.0;
        double s_xxy = 0.0;
        double s_xyy = 0.0;
        for (OffsetT row = 0; row < num_rows; ++row) {
            OffsetT nz_idx_start = row_offsets[row];
            OffsetT nz_idx_end = row_offsets[row + 1];

            for (int nz_idx = nz_idx_start; nz_idx < nz_idx_end; ++nz_idx) {
                OffsetT col = column_indices[nz_idx];
                samples++;
                double x = col;
                double y = row;
                double xy = (x - mean_x) * (y - mean_y);
                double xxy = (x - mean_x) * (x - mean_x) * (y - mean_y);
                double xyy = (x - mean_x) * (y - mean_y) * (y - mean_y);
                double delta;
                delta = xy - s_xy;
                s_xy = s_xy + (delta / samples);
                delta = xxy - s_xxy;
                s_xxy = s_xxy + (delta / samples);
                delta = xyy - s_xyy;
                s_xyy = s_xyy + (delta / samples);
            }
        }
        double s_xx = ss_x / num_nonzeros;
        double s_yy = ss_y / num_nonzeros;
        double deming_slope = (s_yy - s_xx + sqrt(((s_yy - s_xx) * (s_yy - s_xx)) + (4 * s_xy * s_xy))) / (2 * s_xy);
        double pearson_r = (num_nonzeros * s_xy) / (sqrt(ss_x) * sqrt(ss_y));
        fprintf(f, "\t\t\t\t\"pearsonR\": %lf,\n", pearson_r);
        OffsetT cache_line_size = 65536;
        OffsetT cache_lines_num = 64;
        int64_t one_line_load_cnt = 0;
        int64_t one_line_reuse_cnt = 0;
        int64_t cache_load_cnt = 0;
        int64_t cache_reuse_cnt = 0;
        OffsetT col_cache_elements = (num_cols - 1) / cache_line_size + 1;
        OffsetT *cols_cache = (OffsetT *)calloc(col_cache_elements, sizeof(OffsetT));
        double diag_dist_mean = 0.0;
        for (OffsetT row = 0; row < num_rows; ++row) {
            OffsetT nz_idx_start = row_offsets[row];
            OffsetT nz_idx_end = row_offsets[row + 1];
            OffsetT nz = -1;
            for (int nz_idx = nz_idx_start; nz_idx < nz_idx_end; ++nz_idx) {
                OffsetT c = column_indices[nz_idx];
                diag_dist_mean += fabs(row - c);
                if (nz >= 0 && c - nz < cache_line_size) {
                    one_line_reuse_cnt++;
                } else {
                    one_line_load_cnt++;
                    nz = c;
                }
                int cache_line = c / cache_line_size;
                if (cols_cache[cache_line] > 0) {
                    cache_reuse_cnt++;
                } else {
                    cache_load_cnt++;
                    for (int j = 0; j < col_cache_elements; j++) {
                        if (cols_cache[j] > 0) {
                            cols_cache[j]--;
                        }
                    }
                    cols_cache[cache_line] = cache_lines_num;
                }
            }
        }
        free(cols_cache);
        diag_dist_mean /= (double)num_nonzeros;
        double diag_dist_variance = 0.0;
        double diag_dist_skewness = 0.0;
        for (OffsetT row = 0; row < num_rows; ++row) {
            OffsetT nz_idx_start = row_offsets[row];
            OffsetT nz_idx_end = row_offsets[row + 1];
            OffsetT nz = -1;
            for (int nz_idx = nz_idx_start; nz_idx < nz_idx_end; ++nz_idx) {
                OffsetT c = column_indices[nz_idx];
                double dist = fabs(row - c);
                double delta = dist - diag_dist_mean;
                diag_dist_variance += (delta * delta);
                diag_dist_skewness += (delta * delta * delta);
            }
        }
        diag_dist_variance /= num_nonzeros;
        double diag_dist_std_dev = sqrt(diag_dist_variance);
        diag_dist_skewness = (diag_dist_skewness / num_nonzeros) / pow(diag_dist_std_dev, 3.0);
        double diag_dist_variation = diag_dist_std_dev / diag_dist_mean;
        double one_line_load_rate = (double)one_line_load_cnt / (double)num_nonzeros;
        double one_line_reuse_rate = (double)one_line_reuse_cnt / (double)num_nonzeros;
        double cache_load_rate = (double)cache_load_cnt / (double)num_nonzeros;
        double cache_reuse_rate = (double)cache_reuse_cnt / (double)num_nonzeros;
        fprintf(f, "\t\t\t\t\"row\": {\n");
        fprintf(f, "\t\t\t\t\t\"diagDist\": {\n");
        fprintf(f, "\t\t\t\t\t\t\"mean\": %lf,\n", diag_dist_mean);
        fprintf(f, "\t\t\t\t\t\t\"SD\": %lf,\n", diag_dist_std_dev);
        fprintf(f, "\t\t\t\t\t\t\"skewness\": %lf,\n", diag_dist_skewness);
        fprintf(f, "\t\t\t\t\t\t\"variation\": %lf\n", diag_dist_variation);
        fprintf(f, "\t\t\t\t\t}\n");
        fprintf(f, "\t\t\t\t},\n");
        fprintf(f, "\t\t\t\t\"cache\": {\n");
        fprintf(f, "\t\t\t\t\t\"singleLine\": {\n");
        fprintf(f, "\t\t\t\t\t\t\"rates\": {\n");
        fprintf(f, "\t\t\t\t\t\t\t\"load\": %lf,\n", one_line_load_rate);
        fprintf(f, "\t\t\t\t\t\t\t\"reuse\": %lf\n", one_line_reuse_rate);
        fprintf(f, "\t\t\t\t\t\t}\n");
        fprintf(f, "\t\t\t\t\t},\n");
        fprintf(f, "\t\t\t\t\t\"multiLine\": {\n");
        fprintf(f, "\t\t\t\t\t\t\"rates\": {\n");
        fprintf(f, "\t\t\t\t\t\t\t\"load\": %lf,\n", cache_load_rate);
        fprintf(f, "\t\t\t\t\t\t\t\"reuse\": %lf\n", cache_reuse_rate);
        fprintf(f, "\t\t\t\t\t\t}\n");
        fprintf(f, "\t\t\t\t\t}\n");
        fprintf(f, "\t\t\t\t}\n");
        fprintf(f, "\t\t\t}\n");
        fprintf(f, "\t\t}\n");
        fprintf(f, "\t]\n");
        fprintf(f, "}\n");
        fclose(f);
    }

    /**
     * Get graph statistics
     */
    GraphStats Stats() {
        GraphStats stats;
        stats.num_rows = num_rows;
        stats.num_cols = num_cols;
        stats.num_nonzeros = num_nonzeros;

        //
        // Compute diag-distance statistics
        //

        OffsetT samples = 0;
        double mean = 0.0;
        double ss_tot = 0.0;

        for (OffsetT row = 0; row < num_rows; ++row) {
            OffsetT nz_idx_start = row_offsets[row];
            OffsetT nz_idx_end = row_offsets[row + 1];

            for (int nz_idx = nz_idx_start; nz_idx < nz_idx_end; ++nz_idx) {
                OffsetT col = column_indices[nz_idx];
                double x = (col > row) ? col - row : row - col;

                samples++;
                double delta = x - mean;
                mean = mean + (delta / samples);
                ss_tot += delta * (x - mean);
            }
        }

        //
        // Compute deming statistics
        //

        samples = 0;
        double mean_x = 0.0;
        double mean_y = 0.0;
        double ss_x = 0.0;
        double ss_y = 0.0;

        for (OffsetT row = 0; row < num_rows; ++row) {
            OffsetT nz_idx_start = row_offsets[row];
            OffsetT nz_idx_end = row_offsets[row + 1];

            for (int nz_idx = nz_idx_start; nz_idx < nz_idx_end; ++nz_idx) {
                OffsetT col = column_indices[nz_idx];

                samples++;
                double x = col;
                double y = row;
                double delta;

                delta = x - mean_x;
                mean_x = mean_x + (delta / samples);
                ss_x += delta * (x - mean_x);

                delta = y - mean_y;
                mean_y = mean_y + (delta / samples);
                ss_y += delta * (y - mean_y);
            }
        }

        samples = 0;
        double s_xy = 0.0;
        double s_xxy = 0.0;
        double s_xyy = 0.0;
        for (OffsetT row = 0; row < num_rows; ++row) {
            OffsetT nz_idx_start = row_offsets[row];
            OffsetT nz_idx_end = row_offsets[row + 1];

            for (int nz_idx = nz_idx_start; nz_idx < nz_idx_end; ++nz_idx) {
                OffsetT col = column_indices[nz_idx];

                samples++;
                double x = col;
                double y = row;

                double xy = (x - mean_x) * (y - mean_y);
                double xxy = (x - mean_x) * (x - mean_x) * (y - mean_y);
                double xyy = (x - mean_x) * (y - mean_y) * (y - mean_y);
                double delta;

                delta = xy - s_xy;
                s_xy = s_xy + (delta / samples);

                delta = xxy - s_xxy;
                s_xxy = s_xxy + (delta / samples);

                delta = xyy - s_xyy;
                s_xyy = s_xyy + (delta / samples);
            }
        }

        double s_xx = ss_x / num_nonzeros;
        double s_yy = ss_y / num_nonzeros;

        double deming_slope = (s_yy - s_xx + sqrt(((s_yy - s_xx) * (s_yy - s_xx)) + (4 * s_xy * s_xy))) / (2 * s_xy);

        stats.pearson_r = (num_nonzeros * s_xy) / (sqrt(ss_x) * sqrt(ss_y));

        //
        // Compute row-length statistics
        //

        // Sample mean
        stats.row_length_mean = double(num_nonzeros) / num_rows;
        double variance = 0.0;
        stats.row_length_skewness = 0.0;
        for (OffsetT row = 0; row < num_rows; ++row) {
            OffsetT length = row_offsets[row + 1] - row_offsets[row];
            double delta = double(length) - stats.row_length_mean;
            variance += (delta * delta);
            stats.row_length_skewness += (delta * delta * delta);
        }
        variance /= num_rows;
        stats.row_length_std_dev = sqrt(variance);
        stats.row_length_skewness = (stats.row_length_skewness / num_rows) / pow(stats.row_length_std_dev, 3.0);
        stats.row_length_variation = stats.row_length_std_dev / stats.row_length_mean;

        return stats;
    }

    /**
     * Display log-histogram to stdout
     */
    void DisplayHistogram() {
        // Initialize
        OffsetT log_counts[9];
        for (OffsetT i = 0; i < 9; i++) {
            log_counts[i] = 0;
        }

        // Scan
        OffsetT max_log_length = -1;
        OffsetT max_length = -1;
        for (OffsetT row = 0; row < num_rows; row++) {
            OffsetT length = row_offsets[row + 1] - row_offsets[row];
            if (length > max_length)
                max_length = length;

            OffsetT log_length = -1;
            while (length > 0) {
                length /= 10;
                log_length++;
            }
            if (log_length > max_log_length) {
                max_log_length = log_length;
            }

            log_counts[log_length + 1]++;
        }
        printf("CSR matrix (%d rows, %d columns, %d non-zeros, max-length %d):\n", (int)num_rows, (int)num_cols,
               (int)num_nonzeros, (int)max_length);
        for (OffsetT i = -1; i < max_log_length + 1; i++) {
            printf("\tDegree 1e%d: \t%d (%.2f%%)\n", i, log_counts[i + 1], (float)log_counts[i + 1] * 100.0 / num_cols);
        }
        fflush(stdout);
    }

    /**
     * Display matrix to stdout
     */
    void Display() {
        printf("Input Matrix (%d vertices, %d nonzeros):\n", (int)num_rows, (int)num_nonzeros);
        for (OffsetT row = 0; row < num_rows; row++) {
            printf("%d [@%d, #%d]: ", row, row_offsets[row], row_offsets[row + 1] - row_offsets[row]);
            for (OffsetT col_offset = row_offsets[row]; col_offset < row_offsets[row + 1]; col_offset++) {
                printf("%d (%f), ", column_indices[col_offset], values[col_offset]);
            }
            printf("\n");
        }
        fflush(stdout);
    }

    void getHeatMapColor(float value, float *red, float *green, float *blue) {
        const int NUM_COLORS = 4;
        static float color[NUM_COLORS][3] = {{0, 0, 1}, {0, 1, 0}, {1, 1, 0}, {1, 0, 0}};
        // A static array of 4 colors:  (blue,   green,  yellow,  red) using {r,g,b} for each.

        int idx1;               // |-- Our desired color will be between these two indexes in "color".
        int idx2;               // |
        float fractBetween = 0; // Fraction between "idx1" and "idx2" where our value is.

        if (value <= 0) {
            idx1 = idx2 = 0;
        } // accounts for an input <=0
        else if (value >= 1) {
            idx1 = idx2 = NUM_COLORS - 1;
        } // accounts for an input >=0
        else {
            value = value * (NUM_COLORS - 1);   // Will multiply value by 3.
            idx1 = floor(value);                // Our desired color will be after this index.
            idx2 = idx1 + 1;                    // ... and before this index (inclusive).
            fractBetween = value - float(idx1); // Distance between the two indexes (0-1).
        }

        *red = (color[idx2][0] - color[idx1][0]) * fractBetween + color[idx1][0];
        *green = (color[idx2][1] - color[idx1][1]) * fractBetween + color[idx1][1];
        *blue = (color[idx2][2] - color[idx1][2]) * fractBetween + color[idx1][2];
    }
    /**
     * Save matrix as PPM image
     */
    void SavePPM(string ppm_file_name, int max_size = 1024) {
        printf("SavePPM(%s)\n", ppm_file_name.c_str());
        fflush(stdout);
        printf("  max_size=%d\n", max_size);
        fflush(stdout);
        printf("  num_rows=%d\n", num_rows);
        fflush(stdout);
        printf("  num_cols=%d\n", num_cols);
        fflush(stdout);
        printf("  num_nonzeros=%d\n", num_nonzeros);
        fflush(stdout);
        printf("  sizeof(OffsetT)=%d\n", sizeof(OffsetT));
        fflush(stdout);
        printf("  sizeof(ValueT)=%d\n", sizeof(ValueT));
        fflush(stdout);
        int w = 0;
        int h = 0;
        if (num_cols <= max_size && num_rows <= max_size) {
            w = num_cols;
            h = num_rows;
        } else {
            if (num_cols >= num_rows) {
                w = max_size;
                h = (int)((double)max_size * num_rows / num_cols);
            } else {
                h = max_size;
                w = (int)((double)max_size * num_cols / num_rows);
            }
        }
        printf("  w=%d\n", w);
        fflush(stdout);
        printf("  h=%d\n", h);
        fflush(stdout);
        int *arr = new int[w * h];
        memset(arr, 0, sizeof(int) * w * h);

        for (OffsetT row = 0; row < num_rows; row++) {
            int y = row * h / num_rows;
            for (OffsetT col_offset = row_offsets[row]; col_offset < row_offsets[row + 1]; col_offset++) {
                OffsetT col = column_indices[col_offset];
                int x = col * w / num_cols;
                arr[y * w + x]++;
            }
        }

        int max_cnt = 0;
        for (int i = 0; i < w * h; i++) {
            if (max_cnt < arr[i]) {
                max_cnt = arr[i];
            }
        }

        FILE *ppm_file = fopen(ppm_file_name.c_str(), "wt");
        fprintf(ppm_file, "P3\n");
        fprintf(ppm_file, "%d %d\n", w + 6 + 40 + 100, h + 6);
        fprintf(ppm_file, "255\n");
        for (int y = -3; y < h + 3; y++) {
            for (int x = -3; x < w + 3 + 40 + 100; x++) {
                float r = 0, g = 0, b = 0;
                if (x >= 0 && x < w && y >= 0 && y < h) {
                    if (arr[y * w + x] > 0) {
                        getHeatMapColor((float)arr[y * w + x] / max_cnt, &r, &g, &b);
                    }
                } else {
                    if (x < w + 3 || (x >= w + 3 + 20 && x < w + 3 + 40 && (y < 0 || y >= h))) {
                        r = 1;
                        g = 1;
                        b = 1;
                    } else if (x >= w + 3 + 20 && x < w + 3 + 40 && y >= 0 && y < h) {
                        getHeatMapColor(1.0 - (float)y / h, &r, &g, &b);
                    }
                }
                fprintf(ppm_file, "%d %d %d\n", (int)(r * 255), (int)(g * 255), (int)(b * 255));
            }
        }
        fclose(ppm_file);
        string str = string() + "ppmlabel -x " + to_string(w + 6 + 40 + 10) + " -y " + to_string(20) + " -text " +
                     to_string(max_cnt) + " -x " + to_string(w + 6 + 40 + 10) + " -y " + to_string(h - 1) + " -text " +
                     to_string(1) + " -x " + to_string(w + 6 + 40 + 60) + " -y " + to_string(h / 2) +
                     " -angle 90 -text 'NNZs/pixel' " + ppm_file_name + " >" + ppm_file_name + ".temp && mv -f " +
                     ppm_file_name + ".temp " + ppm_file_name;
        system(str.c_str());
    }
};
