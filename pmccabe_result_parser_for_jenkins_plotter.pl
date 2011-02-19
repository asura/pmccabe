#!/usr/bin/env perl
use strict;
use warnings;
use utf8;

print "修正循環複雑度,循環複雑度,ステートメント数,LOC\n";

my $line = <>;
$line =~ s/[\r\n]+$//;
$line =~ s/n\/a//;
$line =~ s/[ \t]*Total$//;
$line =~ s/[ \t]+/,/g;
print("$line\n");
