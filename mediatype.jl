mutable struct Encoding <: Comparable
    contentType::String
    headers::Dict{String,Header}
    style::String
    explode::Bool
    allowReserved::Bool
end

Encoding() = Encoding("", Dict{String,Header}(), "", false, false)

mutable struct MediaType <: CircularReference
    schema::Schema
    example::Any
    examples::Dict{String,Example}
    encoding::Dict{String,Encoding}
end

MediaType() = MediaType(Schema(), Nothing, Dict{String,Example}(), Dict{String,Encoding}())

function parse!(m::MediaType, data::Dict{Any,Any})
    for (key,value) in data
        if key == "schema" parse!(m.schema, value) end
        if key == "example" m.example = value end
        if key == "examples" parse!(m.examples, value) end
        if key == "encoding" parse!(m.encoding, value) end
    end
end

function parse!(m::Dict{String,MediaType}, data::Dict{Any,Any})
    for (key,value) in data
        mt = MediaType()
        parse!(mt, value)
        m["$key"] = mt
    end
end
