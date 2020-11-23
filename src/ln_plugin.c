/*
 * Copyright (c) 2018-2020 Zhixu Zhao
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifdef LN_PLUGIN

#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <dlfcn.h>
#include <link.h>
#include "ln_msg.h"

int ln_plugin_load(const char *plugin_name)
{
    if (syscall(SYS_getpid) != syscall(SYS_gettid)) {
        ln_msg_warn("cannot load a plugin outside the main thread");
        return 0;
    }

    void *handle;
    int load_flags = RTLD_NOW | RTLD_DEEPBIND;
    static Lmid_t plugin_namespace = LM_ID_BASE;

    if (plugin_namespace == LM_ID_BASE)
        handle = dlmopen(LM_ID_NEWLM, plugin_name, load_flags);
    else
        handle = dlmopen(plugin_namespace, plugin_name, load_flags);
    if (!handle) {
        ln_msg_warn("load plugin '%s' failed: %s", plugin_name, dlerror());
        return 0;
    }

    if (plugin_namespace == LM_ID_BASE) {
        if (dlinfo(handle, RTLD_DI_LMID, &plugin_namespace) == -1) {
            ln_msg_warn("dlinfo for plugin_namespace failed: %s", dlerror());
            plugin_namespace = LM_ID_BASE;
        }
    }

    return 1;
}

#endif  /* LN_PLUGIN */
