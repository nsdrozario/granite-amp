nodes = {
    {
        ["type"] = "overdrive",
        ["state"] = {
            ["gain"]=0,
            ["lpf_freq"]=12000,
            ["hpf_freq"]=300,
            ["algorithm"]="tanh"
        }
    },
    {
        ["type"] = "cabsim",
        ["state"] = {
            ["hpf"] = 60,
            ["lpf"] = 12000,
            ["lowmid"] = {
                ["freq"] = 250,
                ["q"] = 4,
                ["gain"] = 6
            },
            ["mid"] = {
                ["freq"] = 500,
                ["q"] = 4,
                ["gain"] = -12
            },
            ["presence"] = {
                ["freq"] = 1250,
                ["q"] = 12,
                ["gain"] = -6
            }
        }
    }
}

adjlist = {
    [3] = 11,
    [13] = 6
}