package ConfigUtil;

use Text::Wrap qw(wrap);
use File::Basename;

require Exporter;
our @ISA = qw(Exporter);
our @EXPORT = qw(gen_options common_checks set_extra_bins set_module_files config_to_str
                 match_version version_roughly_match
                 err_exit exit_msg warn_msg);

sub gen_options {
    my ($meta_opts, $top_desc) = @_;

    my %options;
    my %getopt_args;
    my @option_usages;
    foreach my $meta_opt (@$meta_opts) {
        my ($opt_name, $var_name, $type, $default, $desc);
        if (ref $meta_opt eq ref []) {
            ($opt_name, $var_name, $type, $default, $desc) = @$meta_opt;
        } elsif (ref $meta_opt eq ref {}) {
            my @keys = qw(opt_name var_name type default desc);
            ($opt_name, $var_name, $type, $default, $desc) = @$meta_opt{@keys};
        }
        $options{$var_name} = $default;
        $getopt_args{"${opt_name}=s"} = gen_arg_action(\%options, $var_name);
        push @option_usages, ["--${opt_name}=${type}", "${desc} [${default}]"];
    }
    my $format_usage = format_usage($top_desc, \@option_usages, 80);
    $getopt_args{help} = sub { print $format_usage; exit 0; };

    my $options_obj = { 'options' => \%options, 'getopt_args' => \%getopt_args,
                        'option_usages' => \@option_usages, 'format_usage' => $format_usage };

    return $options_obj;
}

sub gen_arg_action {
    my ($options, $arg_var_name) = @_;

    my $action = sub {
        my $action_arg = $_[1];
        $options->{$arg_var_name} = $action_arg;
    };

    return $action;
}

sub format_usage {
    my ($top_desc, $option_usages, $line_max) = @_;

    $Text::Wrap::columns = $line_max;
    unshift @$option_usages, ["-h, --help", "print help information"];
    my $arg_len_max = 0;
    foreach (@$option_usages) {
        my $len = length($_->[0]);
        $arg_len_max = $arg_len_max > $len ? $arg_len_max : $len;
    }
    my @usages;
    foreach (@$option_usages) {
        my ($arg, $desc) = @$_;
        my $prefix = "  $arg"." "x($arg_len_max + 2 - length($arg));
        push @usages, wrap($prefix, " "x($arg_len_max + 4), $desc);
    }

    return <<EOC;
Usage: $0 [OPTION[=VALUE]...]
$top_desc

options:
@{[ join "\n", @usages ]}
EOC
}

sub set_extra_bins {
    my ($customs) = @_;
    return if not defined $customs->{EXTRA_BINS} or $customs->{EXTRA_BINS} =~ /^\s*$/;

    $customs->{HAS_EXTRA_BINS} = "yes";
    my @extra_bins = split /\s+/, $customs->{EXTRA_BINS};
    my @basenames = map { basename($_) } @extra_bins;
    my @build_extra_bins = map { "\$(BUILD_BIN_DIR)/$_" } @basenames;
    my @install_extra_bins = map { "\$(INSTALL_BIN_DIR)/$_" } @basenames;
    $customs->{BUILD_EXTRA_BINS} = join " ", @build_extra_bins;
    $customs->{INSTALL_EXTRA_BINS} = join " ", @install_extra_bins;
}

sub set_module_files {
    my ($customs, $module_name) = @_;

    my $sub_dirs = "${module_name}_SUB_DIRS";
    my @srcs;
    if (not defined $customs->{$sub_dirs} or $customs->{$sub_dirs} =~ /^\s*$/) {
        push @srcs, "\$(wildcard *.c)";
        push @srcs, "\$(wildcard *.cc)";
        push @srcs, "\$(wildcard *.cpp)";
        push @srcs, "\$(wildcard *.cu)";
    } else {
        push @srcs, "\$(wildcard *.c) \$(foreach dir,\$($sub_dirs),\$(wildcard \$(dir)/*.c))";
        push @srcs, "\$(wildcard *.cc) \$(foreach dir,\$($sub_dirs),\$(wildcard \$(dir)/*.cc))";
        push @srcs, "\$(wildcard *.cpp) \$(foreach dir,\$($sub_dirs),\$(wildcard \$(dir)/*.cpp))";
        push @srcs, "\$(wildcard *.cu) \$(foreach dir,\$($sub_dirs),\$(wildcard \$(dir)/*.cu))";
    }
    my $files = "${module_name}_FILES";
    my $dep_files = "${module_name}_DEP_FILES";
    $customs->{$files} = \@srcs;
    $customs->{$dep_files} = "\$(OBJDIR)/*.d \$(foreach dir,\$($sub_dirs),\$(OBJDIR)/\$(dir)/*.d)";
}

sub common_checks {
    my ($opts) = @_;

    my $output;
    $output = `gcc --version`;
    if (!defined $output or $output eq "") {
        err_exit("gcc is not installed");
    }

    $output = `make --version`;;
    if (!defined $output or $output eq "") {
        err_exit("make is not installed");
    }

    $output = `pkg-config --version`;;
    if (!defined $output or $output eq "") {
        err_exit("pkg-config is not installed");
    }

    if ($opts->{WITH_CUDNN} eq "yes") {
        if (not $opts->{WITH_CUDA} eq "yes") {
            $opts->{WITH_CUDA} = "yes";
            print ("automatically set --with-cuda=yes\n");
        }
    }

    if ($opts->{WITH_TENSORRT} eq "yes") {
        if (not $opts->{WITH_CUDA} eq "yes") {
            $opts->{WITH_CUDA} = "yes";
            print ("automatically set --with-cuda=yes\n");
        }
        if (not $opts->{WITH_CUDNN} eq "yes") {
            $opts->{WITH_CUDNN} = "yes";
            print ("automatically set --with-cudnn=yes\n");
        }
    }

    if ($opts->{WITH_CUDA} eq "yes") {
        $output = `nvcc --version`;;
        if (!defined $output or $output eq "") {
            err_exit("nvcc is not installed");
        }
    }

    if ($opts->{WITH_PYTHON} eq "yes") {
        $output = `$opts->{PYTHON_CMD} --version 2>&1`;
        if (not defined $output or $output eq "" or
            not version_roughly_match($output, $opts->{PYTHON_VERSION})) {
            warn_msg("PYTHON_CMD $opts->{PYTHON_CMD} is not installed or doesn't match PYTHON_VERSION $opts->{PYTHON_VERSION}, now set to python$opts->{PYTHON_VERSION}");
            $opts->{PYTHON_CMD} = "python$opts->{PYTHON_VERSION}";
            $output = `$opts->{PYTHON_CMD} --version 2>&1`;
            if (!defined $output or $output eq "") {
                err_exit("$opts->{PYTHON_CMD} is not installed");
            }
        }
    }

}

sub config_to_str {
    my ($config) = @_;

    my $str;
    foreach my $key (sort keys %$config) {
        if (ref $config->{$key} eq ref []) {
            map { $str .= "$key += $_\n" } @{$config->{$key}};
            next;
        }
        $str .= "$key ?= $config->{$key}\n";
    }
    return $str;
}

sub get_python_lib_dir {
    my $python_cmd = shift;
    my $dir =  `echo 'import distutils.sysconfig as conf
print(conf.get_python_lib())' | $python_cmd`;
    chomp $dir;
    $dir;
}

sub match_version {
    my $input = shift;
    my $version;
    $version = $1 if $input =~ /([0-9a-z.]+)$/;
}

sub version_roughly_match {
    my $ver1 = &match_version(shift);
    my $ver2 = &match_version(shift);
    if ($ver1 =~ /^$ver2/ or $ver2 =~ /^$ver1/) {
        return 1;
    } else {
        return 0;
    }
}

sub err_exit {
    my $msg = shift;
    die "\e[31mError:\e[0m $msg\n";
}

sub warn_msg {
    my $msg = shift;
    warn "\e[35mWarning:\e[0m $msg\n";
}

# Test code
sub selftest {
    use Data::Dumper;
    use Getopt::Long;
    $Data::Dumper::Indent = 1;

    my $meta_options =
        [
         { opt_name => "build-dir", var_name => "BUILD_DIR", type => "DIR",
           default => "build", desc => "build dir" },
         { opt_name => "install-dir", var_name => "INSTALL_DIR", type => "DIR",
           default => "/usr/local", desc => "install dir" },
         { opt_name => "prefix", var_name => "INSTALL_DIR", type => "DIR",
           default => "/usr/local", desc => "prefix" },
         { opt_name => "pkgconfig-dir", var_name => "PKGCONFIG_DIR", type => "DIR",
           default => "\$(INSTALL_DIR)/lib/pkgconfig", desc => "pkgconfig dir" },
         { opt_name => "python-user", var_name => "PYTHON_USER", type => "BOOL",
           default => "no", desc => "install python packages to user site-package '\$HOME/.local/lib/python<version>/site-packages'" },
        ];
    my $options_obj = gen_options($meta_options, "top desc");
    print "default: \n";
    print Dumper($options_obj);
    GetOptions(%{$options_obj->{getopt_args}}) or die "$options_obj->{format_usage}";
    print "final: \n";
    print Dumper($options_obj);
}

unless (caller) {
    selftest();
}

1;
