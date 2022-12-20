mutable struct Example <: Comparable
    summary::String
    description::String
    value::Any
    externalValue::URI
end

Example() = Example("", "", Nothing, URI())

function parse!(e::Example, data::OrderedDict{Any,Any})
    for (key,value) in data
        if key == "summary" e.summary = value end
        if key == "description" e.description = value end
        if key == "value" e.value = value end
        if key == "externalValue" e.externalValue = URI(value) end
    end
end

function parse!(e::OrderedDict{String,Example}, data::OrderedDict{Any,Any})
    for (key,value) in data
        ex = Example()
        parse!(ex, value)
        e[key] = ex
    end
end
