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

  my $string          = "";
  my $in_text         = 0;
  my $start_line_no   = 0;
  my $in_skipped_prop = 0;
  my $tag = "";
  my $attr = "";
  my $context = "";

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
