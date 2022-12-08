mutable struct Parameter <: Comparable
    ref::String
    name::String
    in::String
    description::String
    required::Bool
    deprecated::Bool
    allowEmptyValue::Bool
    style::String
    explode::Bool
    allowReserved::Bool
    schema::Schema
    example::Any
    examples::Dict{String,Example}
    content::Dict{String,MediaType}
end

Parameter() = Parameter("", "", "", "", false, false, false, "", false, false, Schema(), Nothing,
    Dict{String,Example}(), Dict{String,MediaType}())

function parse!(p::Parameter, data::Dict{Any,Any})
    for (key, value) in data
        if key == "\$ref" p.ref = value end
        if key == "name" p.name = value end
        if key == "in" p.in = value end
        if key == "description" p.description = value end
        if key == "required" p.required = value end
        if key == "deprecated" p.deprecated = value end
        if key == "allowEmptyValue" p.allowEmptyValue = value end
        if key == "style" p.style = value end
        if key == "explode" p.explode = value end
        if key == "allowReserved" p.allowReserved = value end
        if key == "schema" parse!(p.schema, value) end
        if key == "example" p.example = value end
        if key == "examples" parse!(p.examples, value) end
        if key == "content" parse!(p.content, value) end
    end
end

function parse!(v::Vector{Parameter}, data::Vector{Dict{Any,Any}})
    for d in data
        p = Parameter()
        parse!(p, d)
        push!(v, p)
    end
end
