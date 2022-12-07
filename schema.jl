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

mutable struct Schema
    discriminator::Discriminator
    xml::XML
    externalDocs::ExternalDocumentation
end

Schema() = Schema(Discriminator(), XML(), ExternalDocumentation())

function parse!(s::Schema, data::Dict{Any,Any})
    for (key,value) in data
        if key == "discriminator" parse!(s.discriminator, value) end
        if key == "xml" parse!(s.xml, value) end
        if key == "externalDocs" parse!(s.externalDocs, value) end
    end
end
