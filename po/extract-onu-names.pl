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
    if ($_ =~ /^name=(.*)/)
    {
      print "//i18n: file $file_name line $.\n";
      print "// xgettext: no-c-format\n";
      print "i18n($1);\n";
    }
    if ($_ =~ /^desc=(.*)/)
    {
      print "//i18n: file $file_name line $.\n";
      print "// xgettext: no-c-format\n";
      print "i18n($1);\n";
    }
    if ($_ =~ /^countries=(.*)/)
    {
      my @countries = split ",", $1;
      foreach my $country (@countries)
      {
        print "//i18n: file $file_name line $.\n";
        print "// xgettext: no-c-format\n";
        print "i18n($country);\n";
      }
    }
    if ($_ =~ /^nationalities=(.*)/)
    {
      my @nationalities = split ",", $1;
      foreach my $nationality (@nationalities)
      {
        print "//i18n: file $file_name line $.\n";
        print "// xgettext: no-c-format\n";
        print "i18n($nationality);\n";
      }
    }
    if ($_ =~ /^continents=(.*)/)
    {
      my @continents = split ",", $1;
      foreach my $continent (@continents)
      {
        print "//i18n: file $file_name line $.\n";
        print "// xgettext: no-c-format\n";
        print "i18n($continent);\n";
      }
    }
  }
}
