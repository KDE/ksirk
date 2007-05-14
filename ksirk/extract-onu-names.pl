#!/usr/bin/perl -W

use strict;

for my $file_name ( @ARGV )
{
  my $fh;

  unless ( open $fh, "<", $file_name )
  {
    # warn "Failed to open: '$file_name': $!";
    next;
  }

  while ( <$fh> )
  {
    if ($_ =~ /name=(\"[^\"]*\")/)
    {
      print "//i18n: file $file_name line $.\n";
      print "// xgettext: no-c-format\n";
      print "i18n($1);\n";
    }
  }
}
