mutable struct Header{MT<:CircularReference} <: Comparable
    description::String
    required::Bool
    deprecated::Bool
    allowEmptyValue::Bool
    style::String
    explode::Bool
    allowReserved::Bool
    schema::Schema
    example::Any
    examples::OrderedDict{String,Example}
    content::OrderedDict{String,MT}
end

Header() = Header("", false, false, false, "", false, false, Schema(), Nothing, OrderedDict{String,Example}(), OrderedDict{String,MT}())

function parse!(h::Header, data::OrderedDict{Any,Any})
    for (key,value) in data
        if key == "description" h.description = value end
        if key == "required" h.required = value end
        if key == "deprecated" h.deprecated = value end
        if key == "allowEmptyValue" h.allowEmptyValue = value end
        if key == "style" h.style = value end
        if key == "explode" h.explode = value end
        if key == "allowReserved" h.allowReserved = value end
        if key == "schema" parse!(h.schema, value) end
    end
end

function parse!(h::OrderedDict{String,Header}, data::OrderedDict{Any,Any})
    for (key,value) in data
        hdr = Header()
        parse!(hdr, value)
        h[key] = hdr
    end
end