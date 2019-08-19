#! /usr/bin/env python

import sys
import pylightnet as ln

def main(argv):
    ln.lib.init()
    option = ln.option.create(ln.lib.str_array(argv))
    ln.msg.init(option)
    ctx = ln.context.create()
    ln.context.init(ctx, ln.option.get_source(option))

    if (ln.option.get_compile(option)):
        ln.context.compile(ctx, ln.option.get_target(option))

    if not ln.util.streq(ln.option.get_outfile(option), b"!"):
        ln.context.Print(ctx, ln.option.get_outfile(option))

    if ln.option.get_run(option):
        ln.context.load(ctx, ln.option.get_datafile(option))
        ln.context.run(ctx)
        ln.context.unload(ctx)

    ln.context.cleanup(ctx)
    ln.context.free(ctx)
    ln.msg.cleanup()
    ln.option.free(option)

if __name__ == "__main__":
    main(sys.argv)
