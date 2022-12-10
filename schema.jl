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

mutable struct Schema <: Comparable
    discriminator::Discriminator
    xml::XML
    externalDocs::ExternalDocumentation
    ref::String
    type::String
    title::String
    summary::String
    description::String
    required::Vector{String}
    maximum::Union{Number,Nothing}
    exclusiveMaximum::Union{Number,Nothing}
    minimum::Union{Number,Nothing}
    exclusiveMinimum::Union{Number,Nothing}
    maxLength::Union{Number,Nothing}
    minLength::Union{Number,Nothing}
    pattern::String
    maxItems::Union{Number,Nothing}
    minItems::Union{Number,Nothing}
    format::String
    enum::Vector{String}
    allOf::Vector{Any}
    nullable::Union{Bool,Nothing}
    readOnly::Union{Bool,Nothing}
    writeOnly::Union{Bool,Nothing}
    deprecated::Union{Bool,Nothing}
    example::Any
    default::Any
    referenceURI::URI # To track external references.
    properties::OrderedDict{String,Schema}
end

Schema() = Schema(Discriminator(), XML(), ExternalDocumentation(), "", "", "", "", "",
    Vector{String}(), nothing, nothing, nothing, nothing, nothing, nothing, "", nothing, nothing,
    "", Vector{String}(), Vector{Any}(), nothing, nothing, nothing, nothing, "", "",
    URI(), OrderedDict{String,Schema}())

function parse!(s::Schema, data::OrderedDict{Any,Any})
    for (key,value) in data
        if key == "discriminator" parse!(s.discriminator, value) end
        if key == "xml" parse!(s.xml, value) end
        if key == "externalDocs" parse!(s.externalDocs, value) end
        if key == "\$ref" s.ref = value end
        if key == "type" s.type = value end
        if key == "title" s.title = value end
        if key == "summary" s.summary = value end
        if key == "description" s.description = value end
        if key == "required"
            for r in value push!(s.required, r) end
        end
        if key == "maximum" s.maximum = value end
        if key == "exclusiveMaximum" s.exclusiveMaximum = value end
        if key == "minimum" s.minimum = value end
        if key == "exclusiveMinimum" s.exclusiveMinimum = value end
        if key == "maxLength" s.maxLength = value end
        if key == "minLength" s.minLength = value end
        if key == "pattern" s.pattern = value end
        if key == "maxItems" s.maxItems = value end
        if key == "minItems" s.minItems = value end
        if key == "format" s.format = value end
        if key == "enum"
            for r in value push!(s.enum, r) end
        end
        if key == "nullable" s.nullable = value end
        if key == "readOnly" s.readOnly = value end
        if key == "writeOnly" s.writeOnly = value end
        if key == "deprecated" s.deprecated = value end
        if key == "allOf"
            s.allOf = value
            for v in value
                if v isa OrderedDict{Any,Any}
                    sc = Schema()
                    parse!(sc, v)
                    if isempty(s.ref) s.ref = sc.ref end
                    if isempty(s.type) s.type = sc.type end
                    if isempty(s.title) s.title = sc.title end
                    if isempty(s.summary) s.summary = sc.summary end
                    if isempty(s.description) s.description = sc.description end
                    if isempty(s.required) s.required = sc.required end
                    for (k,v) in sc.properties s.properties[k] = v end
                end
            end
        end
        if key == "properties" parse!(s.properties, value) end
    end
end

function parse!(s::OrderedDict{String,Schema}, data::OrderedDict{Any,Any})
    for (key,value) in data
        sc = Schema()
        parse!(sc, value)
        s[key] = sc
    end
end
