//                              -*- Mode: C++ -*-
//
// uC++ Version 6.0.0, Copyright (C) Peter A. Buhr and Richard A. Stroobosscher 1994
//
// main.c --
//
// Author           : Richard A. Stroobosscher
// Created On       : Tue Apr 28 15:25:22 1992
// Last Modified By : Peter A. Buhr
// Last Modified On : Fri Dec  9 13:06:30 2011
// Update Count     : 132
//
// This  library is free  software; you  can redistribute  it and/or  modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software  Foundation; either  version 2.1 of  the License, or  (at your
// option) any later version.
//
// This library is distributed in the  hope that it will be useful, but WITHOUT
// ANY  WARRANTY;  without even  the  implied  warranty  of MERCHANTABILITY  or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
// for more details.
//
// You should  have received a  copy of the  GNU Lesser General  Public License
// along  with this library.
//

#include <iostream>
#include <iomanip>
#include <fstream>
#include <csignal>
#include <cstring>					// strcmp, strncmp, strlen
#include <cstdlib>					// exit

using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::ifstream;
using std::ofstream;

#include "main.h"
#include "key.h"
#include "hash.h"
#include "symbol.h"
#include "token.h"
#include "table.h"
#include "input.h"
#include "output.h"
#include "parse.h"

//#define __U_DEBUG_H__

istream *yyin = &cin;
ostream *yyout = &cout;

bool error = false;
bool Yield = false;
bool verify = false;
bool gnu = false;
bool user = false;
bool profile = false;

extern void sigSegvBusHandler( int sig );

int main( int argc, char *argv[] ) {
    char *infile = NULL;
    char *outfile = NULL;

    //
    // The translator can receive 2 types of arguments.
    //
    // The first type begin with a '-' character and are generally -D<string>
    // type arguments.  We are interested in arguments, -D__U_YIELD__,
    // -D__U_VERIFY__ and __GNUG__ because they affect the code that is
    // produced by the translator.
    //
    // The second type of argument are input and output file specifications.
    // These arguments do not begin with a '-' character.  The first file
    // specification is taken to be the input file specification while the
    // second file specification is taken to be the output file specification.
    // If no files are specified, stdin and stdout are assumed.  If more files
    // are specified, an error results.
    //

    for ( int i = 1; i < argc; i += 1 ) {
#ifdef __U_DEBUG_H__
	cerr << "argv[" << i << "]:\"" << argv[i] << "\"" << endl;
#endif // __U_DEBUG_H__
	if ( argv[i][0] == '-' ) {
	    if ( strcmp( argv[i], "-D__U_YIELD__" ) == 0 ) {
		Yield = true;
	    } else if ( strcmp( argv[i], "-D__U_VERIFY__" ) == 0 ) {
		verify = true;
	    } else if ( strcmp( argv[i], "-D__U_PROFILE__" ) == 0 ) {
		profile = true;
	    } else if ( strncmp( argv[i], "-D__GNUG__", strlen( "-D__GNUG__" ) ) == 0 ) {
		gnu = true;
	    } // if
	} else {
	    if ( infile == NULL ) {
		infile = argv[i];
#ifdef __U_DEBUG_H__
		cerr << "infile:" << infile << endl;
#endif // __U_DEBUG_H__
		yyin = new ifstream( infile );
		if ( yyin->fail() ) {
		    cerr << "uC++ Translator error: could not open file " << infile << " for reading." << endl;
		    exit( EXIT_FAILURE );
		} // if
	    } else if ( outfile == NULL ) {
		outfile = argv[i];
#ifdef __U_DEBUG_H__
		cerr << "outfile:" << outfile << endl;
#endif // __U_DEBUG_H__
		yyout = new ofstream( outfile );
		if ( yyout->fail() ) {
		    cerr << "uC++ Translator error: could not open file " << outfile << " for writing." << endl;
		    exit( EXIT_FAILURE );
		} // if
	    } else {
		cerr << "Usage: " << argv[0] << " [options] [input-file [output-file]]" << endl;
		exit( EXIT_FAILURE );
	    } // if
	} // if
    } // for

    *yyin >> std::resetiosflags( std::ios::skipws );	// turn off white space skipping during input

    signal( SIGSEGV, sigSegvBusHandler );
    signal( SIGBUS,  sigSegvBusHandler );

    // This is the heart of the translator.  Although inefficient, it is very
    // simple.  First, all the input is read and convert to a list of tokens.
    // Second, this list is parsed, extracting and inserting tokens as
    // necessary. Third, this list of tokens is converted into an output stream
    // again.

    hash_table = new hash_table_t;

    focus = root = new table_t( NULL );			// start at the root table
    top = new lexical_t( focus );

    // Insert the keywords into the root symbol table.

    for ( int i = 0; key[i].text != NULL; i += 1 ) {
	hash_table->lookup( key[i].text, key[i].value );
    } // for

    read_all_input();
    translation_unit();					// parse the program
    write_all_output();

#if 0
    root->display_table( 0 );
#endif


// TEMPORARY: deleting this data structure does not work!!!!!!!!!
//    delete root;

    delete hash_table;

    // close any open files before quitting.

    if ( yyin != &cin ) delete yyin;
    if ( yyout != &cout ) delete yyout;

    // If an error has occurred during the translation phase, return a negative
    // result to signify this fact.  This will cause the host compiler to
    // terminate the compilation at this point, just as if the regular cpp had
    // failed.

//    return error ? -1 : 0;
    return 0;
} // main

// Local Variables: //
// compile-command: "make install" //
// End: //