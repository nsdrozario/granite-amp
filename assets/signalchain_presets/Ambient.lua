nodes = {
    {
        ["type"]="Delay",
        ["state"]={
            ["DelayTimeSeconds"]=0.2,
            ["FeedbackGain"]=-6 
        }
    },
    {
        ["type"] = "Overdrive",
        ["state"] = {
            ["Gain"]=3,
            ["Volume"]=-20,
            ["LPF"]=16000,
            ["HPF"]=100,
        }
    },
    {
        ["type"]="Flanger",
        ["state"]={
            ["DelayTimeMilliseconds"]=10,
            ["DelayFrequency"]=0.7,
            ["Depth"]=75,
            ["FeedbackGain"]=-5
        }
    },
    {
        ["type"] = "CabSim",
        ["state"] = {
            ["Delay"]=0.7,
            ["HPF"] = 60,
            ["LPF"] = 8000,
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
    [18] = 21,
    [23] = 26,
    [28] = 6
}

adjlist_inward = {
    [3] = 11,
    [13] = 16,
    [18] = 21,
    [23] = 26,
    [28] = 6
}