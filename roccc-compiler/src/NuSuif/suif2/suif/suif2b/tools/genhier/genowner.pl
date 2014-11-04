#!/usr/bin/perl -w

use strict;

# The stuff is store in a hash of lists
my %children = ();
my %ownedby = ();
my %owns = ();
my %parent = ();


my %print_parent = ();

sub print_ownership_hier {
  my $node = shift;
  my $indent = shift || 0;

  print " " x $indent . $node . "\n";

  for my $subnode (@{$owns{$node}}) {
    if (!defined $print_parent{$subnode}) {
      $print_parent{$subnode} = 1;
      &print_ownership_hier($subnode, $indent + 4);
    }
  }
}

sub print_hier
{
  my $obj = shift;
  my $indent = shift || 0;

  print " " x $indent;
  print $obj . "";
  if (defined $ownedby{$obj}) {
    print " " . "(owners: ";
    print join(' ', @{$ownedby{$obj}}) . ")";
  }

  print "\n";

  if (defined $children{$obj}) {
    foreach my $subclass (sort @ {$children{$obj}}) {
      &print_hier($subclass, $indent + 4);
    }
  }
}


my $current = undef;
my @nodes = ();

while (<>) {
  # Get rid of comments
  s/\#.*//;

  if (/(\w+)\s*(\[.*\])?\s*:\s*(\w+)/) {
    $current = $1;
    push @nodes, $current;

    $parent{$1} = $3;
    $owns{$1} = [];

    if (!defined $children{$3}) {
      $children{$3} = [ $1 ];
    } else {
      push @ { $children{$3} }, $1;
    }
  }

  if (/(\w+)\s*\*\s*owner.*\s([\w_]+)\s*;/) {
    my $type_name = $1;

    push @{$owns{$current}}, $type_name;

    if (!defined $ownedby{$type_name}) {
      $ownedby{$type_name} = [ $current ];
    } else {

      push @ { $ownedby{$type_name} }, $current;
    }

    print $current . " -> " . $type_name . " [label = " . $2 . "]\n";
  }

  if (/SourceOp.*\s([\w_]+)\s*;/) {
    
    print $current . " -> " . "SourceOp" . " [label = ". $1 . "]\n";
    push @ { $ownedby{"Expression"} }, $current;
  }

  if (/^\}/) {
    $current = undef;
  }
}

# Post-processing

for my $node (keys %ownedby) {
  my %nodehash = ();

  for my $owner (@{$ownedby{$node}}) {
    $nodehash{$owner} = 1;
  }

  $ownedby{$node} = [] ;

  for my $owner (sort keys %nodehash) {
    push @{$ownedby{$node}}, $owner;
  }
}

&print_hier("SuifObject");
#&print_ownership_hier("FileSetBlock");



