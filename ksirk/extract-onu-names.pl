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
    chomp;
    if ($_ eq "[cannon]"
      || $_ eq "[cavalry]"
      || $_ eq "[infantry]"
      || $_ eq "[exploding]"
      || $_ eq "[firing]"
      || $_ eq "[flag]"
      || $_ eq "[font]"
      || $_ eq "[onu]"
      || $_ =~ /\[goal\d+\]/
      )
    {
#       print "$_: next!\n";
      next;
    }
    elsif ($_ =~ /\[([^\]]*)\]/)
    {
      print "//i18n: file $file_name line $.\n";
      print "// xgettext: no-c-format\n";
      print "i18n(\"$1\");\n";
    }
    elsif ($_ =~ /^(\w+)=(.*)/)
    {
#       print "====== $1  =  $2\n";
      if ($1 eq "desc" || $1 eq "name")
      {
        print "//i18n: file $file_name line $.\n";
        print "// xgettext: no-c-format\n";
        print "i18n(\"$2\");\n";
      }
      elsif ($1 eq "continents" || $1 eq "countries" || $1 eq "nationalities")
      {
        my @strings = split ",", $2;
        foreach my $str (@strings)
        {
          print "//i18n: file $file_name line $.\n";
          print "// xgettext: no-c-format\n";
          print "i18n(\"$str\");\n";
        }
      }
    }
    else
    {
#       print "unhandled\n";
    }
  }
}
