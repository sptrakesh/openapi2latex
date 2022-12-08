mutable struct Discriminator
    propertyName::String
    mapping::Dict{String,String}
end

Discriminator() = Discriminator("", Dict{String,String}())

function parse!(d::Discriminator, data::Dict{Any,Any})
    for (key,value) in data
        if key == "propertyName" s.propertyName = value end
        if key == "mapping"
            for (k,v) in value d.mapping[k] = v end
        end
    end
end

mutable struct XML
    name::String
    namespace::String
    prefix::String
    attribute::Bool
    wrapped::Bool
end

XML() = XML("", "", "", false, false)

function parse!(x::XML, data::Dict{Any,Any})
    for (key,value) in data
        if key == "name" s.name = value end
        if key == "namespace" s.namespace = value end
        if key == "prefix" s.prefix = value end
        if key == "attribute" s.attribute = value end
        if key == "wrapped" s.wrapped = value end
    end
end

mutable struct Property
    ref::String
    type::String
    summary::String
    description::String
    default::Any
    example::Any
    enum::Vector{String}
end

Property() = Property("", "", "", "", "", "", Vector{String}())

function parse!(p::Property, data::Dict{Any,Any})
    for (key,value) in data
        if key == "\ref" p.ref = value end
        if key == "type" p.type = value end
        if key == "summary" p.summary = value end
        if key == "description" p.description = value end
        if key == "default" p.default = value end
        if key == "example" p.example = value end
        if key == "enum"
            for e in value push!(p.enum, e) end
        end
    end
end

function parse!(p::Dict{String,Property}, data::Dict{Any,Any})
    for (key,value) in data
        pr = Property()
        parse!(pr, value)
        p[key] = pr
    end
end

mutable struct Schema
    discriminator::Discriminator
    xml::XML
    externalDocs::ExternalDocumentation
    ref::String
    type::String
    summary::String
    description::String
    required::Vector{String}
    properties::Dict{String,Property}
end

Schema() = Schema(Discriminator(), XML(), ExternalDocumentation(), "", "", "", "", Vector{String}(), Dict{String,Property}())

function parse!(s::Schema, data::Dict{Any,Any})
    for (key,value) in data
        if key == "discriminator" parse!(s.discriminator, value) end
        if key == "xml" parse!(s.xml, value) end
        if key == "externalDocs" parse!(s.externalDocs, value) end
        if key == "\$ref" s.ref = value end
        if key == "type" s.type = value end
        if key == "summary" s.summary = value end
        if key == "description" s.description = value end
        if key == "required"
            for r in value push!(s.required, r) end
        end
        if key == "properties" parse!(s.properties, value) end
    end
end
