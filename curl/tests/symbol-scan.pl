#!/usr/bin/env perl
#***************************************************************************
#                                  _   _ ____  _
#  Project                     ___| | | |  _ \| |
#                             / __| | | | |_) | |
#                            | (__| |_| |  _ <| |___
#                             \___|\___/|_| \_\_____|
#
# Copyright (C) 2010-2011, Daniel Stenberg, <daniel@haxx.se>, et al.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at http://curl.haxx.se/docs/copyright.html.
#
# You may opt to use, copy, modify, merge, publish, distribute and/or sell
# copies of the Software, and permit persons to whom the Software is
# furnished to do so, under the terms of the COPYING file.
#
# This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
# KIND, either express or implied.
#
###########################################################################
#
# This script grew out of help from Przemyslaw Iskra and Balint Szilakszi
# a late evening in the #curl IRC channel on freenode.
#

use strict;
use warnings;

# we may get the dir root pointed out
my $root=$ARGV[0] || ".";

# need an include directory when building out-of-tree
my $i = ($ARGV[1]) ? "-I$ARGV[1] " : '';

my $h = "$root/include/curl/curl.h";
my $mh = "$root/include/curl/multi.h";

my $verbose=0;
my $summary=0;
my $misses=0;

my @syms;
my %doc;
my %rem;

open H_IN, "-|", "cc -E $i$h" || die "Cannot preprocess curl.h";
while ( <H_IN> ) {
    if ( /enum\s+(\S+\s+)?{/ .. /}/ ) {
        s/^\s+//;
        next unless /^CURL/;
        chomp;
        s/[,\s].*//;
        push @syms, $_;
    }
}
close H_IN || die "Error preprocessing curl.h";

sub scanheader {
    my ($f)=@_;
    open H, "<$f";
    while(<H>) {
        if (/^#define (CURL[A-Za-z0-9_]*)/) {
            push @syms, $1;
        }
    }
    close H;
}

scanheader($h);
scanheader($mh);

open S, "<$root/docs/libcurl/symbols-in-versions";
while(<S>) {
    if(/(^CURL[^ \n]*) *(.*)/) {
        my ($sym, $rest)=($1, $2);
        if($doc{$sym}) {
            print "Detected duplicate symbol: $sym\n";
            $misses++;
            next;
        }
        $doc{$sym}=$sym;
        my @a=split(/ +/, $rest);
        if($a[2]) {
            # this symbol is documented to have been present the last time
            # in this release
            $rem{$sym}=$a[2];
        }
    }
}
close S;

my $ignored=0;
for my $e (sort @syms) {
    # OBSOLETE - names that are just placeholders for a position where we
    # previously had a name, that is now removed. The OBSOLETE names should
    # never be used for anything.
    #
    # CURL_EXTERN - is a define used for libcurl functions that are external,
    # public. No app or other code should ever use it.
    #
    # *_LAST and *_LASTENTRY are just prefix for the placeholders used for the
    # last entry in many enum series.
    #

    if($e =~ /(OBSOLETE|^CURL_EXTERN|_LAST\z|_LASTENTRY\z)/) {
        $ignored++;
        next;
    }
    if($doc{$e}) {
        if($verbose) {
            print $e."\n";
        }
        $doc{$e}="used";
        next;
    }
    else {
        print $e."\n";
        $misses++;
    }
}

#
# now scan through all symbols that were present in the symbols-in-versions
# but not in the headers
#
# If the symbols were marked 'removed' in symbols-in-versions we don't output
# anything about it since that is perfectly fine.
#

my $anyremoved;

for my $e (sort keys %doc) {
    if(($doc{$e} ne "used") && !$rem{$e}) {

        if(!$anyremoved++) {
            print "Missing symbols mentioned in symbols-in-versions\n";
            print "Add them to a header, or mark them as removed.\n";
        }

        print "$e\n";
        $misses++;
    }
}

if($summary) {
    print "Summary:\n";
    printf "%d symbols in headers (out of which %d are ignored)\n", scalar(@syms),
    $ignored;
    printf "%d symbols in headers are interesting\n",
    scalar(@syms)- $ignored;
    printf "%d symbols are listed in symbols-in-versions\n (out of which %d are listed as removed)\n", scalar(keys %doc), scalar(keys %rem);
    printf "%d symbols in symbols-in-versions should match the ones in headers\n", scalar(keys %doc) - scalar(keys %rem);
}

if($misses) {
    exit 2; # there are stuff to attend to!
}
