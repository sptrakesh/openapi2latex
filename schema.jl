mutable struct Discriminator <: Comparable
    propertyName::String
    mapping::OrderedDict{String,String}
end

Discriminator() = Discriminator("", OrderedDict{String,String}())

function parse!(d::Discriminator, data::OrderedDict{Any,Any})
    for (key,value) in data
        if key == "propertyName" s.propertyName = value end
        if key == "mapping"
            for (k,v) in value d.mapping[k] = v end
        end
    end
end

mutable struct XML <: Comparable
    name::String
    namespace::String
    prefix::String
    attribute::Bool
    wrapped::Bool
end

XML() = XML("", "", "", false, false)

function parse!(x::XML, data::OrderedDict{Any,Any})
    for (key,value) in data
        if key == "name" s.name = value end
        if key == "namespace" s.namespace = value end
        if key == "prefix" s.prefix = value end
        if key == "attribute" s.attribute = value end
        if key == "wrapped" s.wrapped = value end
    end
end

mutable struct Property <: Comparable
    ref::String
    type::String
    summary::String
    description::String
    default::Any
    example::Any
    enum::Vector{String}
    referenceURI::URI # To track external references.
end

Property() = Property("", "", "", "", "", "", Vector{String}(), URI())

function parse!(p::Property, data::OrderedDict{Any,Any})
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

function parse!(p::OrderedDict{String,Property}, data::OrderedDict{Any,Any})
    for (key,value) in data
        pr = Property()
        parse!(pr, value)
        p[key] = pr
    end
end

mutable struct Schema <: Comparable
    discriminator::Discriminator
    xml::XML
    externalDocs::ExternalDocumentation
    ref::String
    type::String
    summary::String
    description::String
    required::Vector{String}
    properties::OrderedDict{String,Property}
    allOf::Vector{Any}
    referenceURI::URI # To track external references.
end

Schema() = Schema(Discriminator(), XML(), ExternalDocumentation(), "", "", "", "",
    Vector{String}(), OrderedDict{String,Property}(), Vector{Any}(), URI())

function parse!(s::Schema, data::OrderedDict{Any,Any})
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
        if key == "allOf"
            s.allOf = value
            for v in value
                if v isa OrderedDict{Any,Any}
                    sc = Schema()
                    parse!(sc, v)
                    if isempty(s.ref) s.ref = sc.ref end
                    if isempty(s.type) s.type = sc.type end
                    if isempty(s.summary) s.summary = sc.summary end
                    if isempty(s.description) s.description = sc.description end
                    if isempty(s.required) s.required = sc.required end
                    for (k,v) in sc.properties s.properties[k] = v end
                end
            end
        end
    end
end
