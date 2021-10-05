nodes = {
    {
        ["type"] = "Overdrive",
        ["state"] = {
            ["Gain"]=0,
            ["Volume"]=-35,
            ["LPF"]=12000,
            ["HPF"]=300,
        }
    },
    {
        ["type"] = "CabSim",
        ["state"] = {
            ["HPF"] = 60,
            ["LPF"] = 2000,
            ["LowMid"] = {
                ["freq"] = 250,
                ["q"] = 4,
                ["gain"] = 6
            },
            ["Mid"] = {
                ["freq"] = 500,
                ["q"] = 4,
                ["gain"] = -12
            },
            ["Presence"] = {
                ["freq"] = 1250,
                ["q"] = 12,
                ["gain"] = -6
            }
        }
    },
}

adjlist = {
    [3] = 11,
    [13] = 16,
    [18] = 6
}

adjlist_inward = {
    [3] = 11,
    [13] = 16,
    [18] = 6
}