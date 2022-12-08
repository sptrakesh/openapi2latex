mutable struct Encoding <: Comparable
    contentType::String
    headers::OrderedDict{String,Header}
    style::String
    explode::Bool
    allowReserved::Bool
end

Encoding() = Encoding("", OrderedDict{String,Header}(), "", false, false)

mutable struct MediaType <: CircularReference
    schema::Schema
    example::Any
    examples::OrderedDict{String,Example}
    encoding::OrderedDict{String,Encoding}
end

MediaType() = MediaType(Schema(), Nothing, OrderedDict{String,Example}(), OrderedDict{String,Encoding}())

function parse!(m::MediaType, data::OrderedDict{Any,Any})
    for (key,value) in data
        if key == "schema" parse!(m.schema, value) end
        if key == "example" m.example = value end
        if key == "examples" parse!(m.examples, value) end
        if key == "encoding" parse!(m.encoding, value) end
    end
end

function parse!(m::OrderedDict{String,MediaType}, data::OrderedDict{Any,Any})
    for (key,value) in data
        mt = MediaType()
        parse!(mt, value)
        m["$key"] = mt
    end
end
