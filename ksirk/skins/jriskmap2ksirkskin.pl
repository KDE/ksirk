#!/usr/bin/perl -W

use strict;

my @countries;
my @continents;
my %files;

my $inFiles = 0;
my $inContinents = 0;
my $inCountries = 0;
my $inBorders = 0;

while (<>)
{
  chomp;
  if (/^\s*$/) 
  {
    next;
  }
  if (/\[files\]/)
  {
    $inFiles = 1;
    $inContinents = 0;
    $inCountries = 0;
    $inBorders = 0;
  }
  elsif (/\[continents\]/)
  {
    $inFiles = 0;
    $inContinents = 1;
    $inCountries = 0;
    $inBorders = 0;
  }
  elsif (/\[countries\]/)
  {
    $inFiles = 0;
    $inContinents = 0;
    $inCountries = 1;
    $inBorders = 0;
  }
  elsif (/\[borders\]/)
  {
    $inFiles = 0;
    $inContinents = 0;
    $inCountries = 0;
    $inBorders = 1;
  }
  elsif ($inFiles)
  {
    if (/([^\s]+)\s+([^\s]+)/)
    {
      $files{$1} = $2;
    }
  }
  elsif ($inContinents)
  {
    if (/([^\s]+)\s+([^\s]+)\s+([^\s]+)/)
    {
      my %continent;
      $continent{"name"} = $1;
      $continent{"bonus"} = $2;
      $continent{"color"} = $1;
      push @continents, \%continent;
    }
  }
  elsif ($inCountries)
  {
    if (/([^\s]+)\s+([^\s]+)\s+([^\s]+)\s+([^\s]+)\s+([^\s]+)/)
    {
      my %country;
      $country{"name"} = $2;
      push @{${$continents[$3-1]}{"countries"}}, $1;
      $country{"continent"} = $3;
      $country{"x"} = int($4 * 1.5);
      $country{"y"} = int($5 * 1.5);
      my @bords;
      $country{"borders"} = \@bords;
      $countries[$1] = \%country;
    }
  }
  elsif ($inBorders)
  {
    if ($_ !~ /^$/)
    {
      my @tab = split(/ /, $_);
      my $country = shift @tab;
      ${$countries[$country]}{"borders"} = \@tab;
    }
  }
}

for (my $i=1; $i <= $#countries; $i++)
{
  my %country = %{$countries[$i]};
#   print "$country{'name'}\n";
  my ($canx, $cany, $infx, $infy, $cavx, $cavy);
  $canx = $country{'x'}-30;
  $cany = $country{'y'}-25;
  $infx = $country{'x'}-25;
  $infy = $country{'y'}+5;
  $cavx = $country{'x'}+15;
  $cavy = $country{'y'}-15;
  print <<EOF;
  <country id="$i" name="$country{'name'}" >
    <flag-point x="$country{'x'}" y="$country{'y'}" />
    <central-point x="$country{'x'}" y="$country{'y'}" />
    <cannons-point x="$canx" y="$cany" />
    <cavalry-point x="$cavx" y="$cavy" />
    <infantry-point x="$infx" y="$infy" />
    <neighbours>
EOF

  my $neighbour;
  foreach $neighbour ( @{ $country{"borders"} } )
  {
    print "      <neighbour id=\"$neighbour\" />\n";
  }
  
  print <<EOF;
    </neighbours>
  </country>
EOF
}


for (my $i=0; $i <= $#continents; $i++)
{
  my $id = $i+1;
  my %continent = %{$continents[$i]};
  print "  <continent id=\"$id\" name=\"$continent{name}\" bonus=\"$continent{bonus}\" >\n";
  foreach my $country (@{$continent{"countries"}})
  {
    print "    <continent-country id=\"$country\" />\n";
  }

  print "  </continent>\n";
}
