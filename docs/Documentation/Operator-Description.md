# Operator Description

LightNet can generate operator defination code from Operator Descriptions
automatically, which is conforming to the following format:

    :::js
    {
        "ops": [
            ...
            {
                "optype": STRING,
                "author": STRING,
                "arch": STRING,
                "autogen": OPTIONAL BOOL
                "extra_privs": OPTIONAL [
                    {
                        "type": STRING,
                        "name": STRING,
                    },
                    ...
                ],
                "tensors_in": [
                    {
                        "arg_name": STRING,
                        "mtype": STRING, 
                        "ndim": OPTIONAL NUMBER or STRING,
                        "len": OPTIONAL NUMBER or STRING,
                        "sametype": OPTIONAL STRING,
                        "sameshape": OPTIONAL STRING,
                        "static": OPTIONAL BOOL,
                        "check": OPTIONAL STRING,
                        "checks": OPTIONAL [
                            "check": OPTIONAL STRING,
                            ...
                        ],
                        "custom": OPTIONAL STRING,
                        ...
                    },
                    ...
                ],
                "tensors_out": [
                    {
                        "arg_name": STRING,
                        "mtype": STRING,
                        "ndim": OPTIONAL NUMBER or STRING, 
                        "dims": OPTIONAL STRING,
                        "len": OPTIONAL NUMBER or STRING,
                        "dtype": OPTIONAL STRING,
                        "custom": OPTIONAL STRING,
                        "owner": OPTIONAL STRING,
                        "static": OPTIONAL BOOL,
                        "mtype": OPTIONAL STRING,
                        "cleanup": OPTIONAL STRING,
                    },
                    ...
                ],
                "params": [
                    {
                        "arg_name": STRING,
                        "ptype": STRING,
                        "realtype": OPTIONAL STRING,
                        "from_func": OPTIONAL STRING,
                        "eq": OPTIONAL NUMBER or STRING,
                        "gt": OPTIONAL NUMBER or STRING,
                        "ge": OPTIONAL NUMBER or STRING,
                        "lt": OPTIONAL NUMBER or STRING,
                        "le": OPTIONAL NUMBER or STRING,
                        "ne": OPTIONAL NUMBER or STRING,
                        "check": OPTIONAL STRING,
                        "checks": OPTIONAL [
                            "check": OPTIONAL STRING,
                            ...
                        ],
                        "custom": OPTIONAL STRING,
                    },
                    ...
                ],
                "custom": OPTIONAL STRING,
                "static_run": OPTIONAL STRING,
                "run": OPTIONAL STRING,
                "post_run": OPTIONAL STRING,
                "calc_offset": OPTIONAL STRING,
                ...
            },
            ...
        ]
    }
