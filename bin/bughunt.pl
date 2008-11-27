#!/usr/bin/perl

sub bracket_error;
sub run;

our $hi = shift @ARGV;
our $lo = 0;
our $cmd = join " ",@ARGV;

our $good_result = run($lo);

if (run($hi) == $good_result) { 
    print "ok\n";
    exit 0;
}

my $pos = bracket_error($lo,$hi);

printf "Running to %s\n", $pos + 1;
print (run ($pos + 1));
print "Error at $pos\n";
exit 0;

# binary search for the instruction that fails

sub bracket_error ($$) { 
    my ($lo, $hi) = @_;
    my $mid = ($hi + ($hi - $lo)/2);
    while ($lo <= $hi) { 
	if (run($mid) == $good_result) {
	    $hi = $mid - 1;
	} else { 
	    $lo = $mid + 1;
	}
        $mid = ($hi + ($hi - $lo)/2);
    }
    return $mid;
}



sub run ($) { 
    my $steps = shift;
    open(TEST,"setarch x86_64 -R $cmd --jitpp_steps=$steps|") || die "Unable to run program";
    my @lines = <TEST>;
    close(TEST);
    my $result = join "",@lines;
    print "output for $steps\n$result";
    return $result;
}
