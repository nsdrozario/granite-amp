--[[
    DO NOT EDIT THIS FILE!

    This file is a critical module for this application and editing it could result in
    undesirable behavior, including but not limited to unintended changes to your file system.
]]

function split(str, delim) 
    local t = {}
    for substr in string.gmatch(str, "[^" .. delim .. "]+") do
        table.insert(t, substr)
    end
    return t
end

function read_config_file(file_contents)
    local obj = {};
    for i, l in ipairs(split(file_contents, "\n")) do
        local key, value = string.match(l, "(%w+): ([%-%w]+)");
        if key and value then
            value = tonumber(value);
            obj[key] = value;
        end
    end
    return obj;
end

function serialize(t)
    local str = "";
    for k,v in pairs(t) do
        str = str .. k .. ": " .. v;
        str = str .. "\n";
    end
    return str;
end