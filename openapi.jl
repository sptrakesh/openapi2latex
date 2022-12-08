import YAML: YAMLDocIterator, iterate, load_all_file
import URIs: uristring

mutable struct Tag <: Comparable
    name::String
    description::String
    externalDocs::ExternalDocumentation
end

Tag() = Tag("", "", ExternalDocumentation())

function parse!(t::Tag, data::Dict{Any,Any})
    for (key,value) in data
        if key == "name" t.name = value end
        if key == "description" t.description = convert(value) end
        if key == "externalDocs" parse!(t.externalDocs, value) end
    end
end

function parse!(v::Vector{Tag}, data::Vector{Dict{Any,Any}})
    for d in data
        t = Tag()
        parse!(t, d)
        push!(v, t)
    end
end

mutable struct Components <: Comparable
    schemas::Dict{String,Schema}
    responses::Dict{String,Response}
    parameters::Dict{String,Parameter}
    examples::Dict{String,Example}
    requestBodies::Dict{String,RequestBody}
    headers::Dict{String,Header}
    securitySchemes::Dict{String,SecurityScheme}
end

Components() = Components(Dict{String,Schema}(), Dict{String,Response}(), Dict{String,Parameter}(),
    Dict{String,Example}(), Dict{String,RequestBody}(), Dict{String,Header}(), Dict{String,SecurityScheme}())

function parse!(c::Components, data::Dict{Any,Any})
    for (key,value) in data
        if key == "schemas"
            for (k,v) in value
                s = Schema()
                parse!(s, v)
                c.schemas[k] = s
            end
        end
        if key == "responses"
            for (k,v) in value
                r = Response()
                parse!(r, v)
                c.responses[k] = r
            end
        end
        if key == "parameters"
             for (k,v) in value
                 p = Parameter()
                 parse!(p, v)
                 c.parameters[k] = p
             end
         end
        if key == "example"
            for (k,v) in value
                e = Example()
                parse!(e, v)
                c.examples[k] = e
            end
        end
        if key == "requestBodies"
            for (k,v) in value
                r = RequestBody()
                parse!(r, v)
                c.requestBodies[k] = r
            end
        end
        if key == "headers"
            for (k,v) in value
                h = Header()
                parse!(h, v)
                c.headers[k] = h
            end
        end
        if key == "securitySchemes"
            for (k,v) in value
                s = SecurityScheme()
                parse!(s, v)
                c.securitySchemes[k] = s
            end
        end
    end
end

mutable struct TagGroup <: Comparable
    name::String
    tags::Vector{String}
end

TagGroup() = TagGroup("", Vector{String}())

function parse!(t::TagGroup, data::Dict{Any,Any})
    for (key,value) in data
        if key == "name" t.name = value end
        if key == "tags"
            for tg in value push!(t.tags, tg) end
        end
    end
end

function parse!(t::Vector{TagGroup}, data::Vector{Dict{Any,Any}})
    for d in data
        tg = TagGroup()
        parse!(tg, d)
        push!(t, tg)
    end
end

mutable struct OpenAPI <: Comparable
    openapi::String
    info::Info
    jsonSchemaDialect::String
    servers::Vector{Server}
    paths::Dict{String,PathItem}
    webhooks::Dict{String,PathItem}
    components::Components
    security::Vector{SecurityRequirement}
    tags::Vector{Tag}
    externalDocs::ExternalDocumentation
    tagGroups::Vector{TagGroup}
end

OpenAPI() = OpenAPI("", Info(), "", Vector{Server}(), Dict{String,PathItem}(), Dict{String,PathItem}(),
    Components(), Vector{SecurityRequirement}(), Vector{Tag}(), ExternalDocumentation(), Vector{TagGroup}())

function parse(it::YAMLDocIterator)::OpenAPI
    (yaml, state) = iterate(it)
    @debug "YAML iterator state: $state"

    api = OpenAPI()

    for (key, value) in yaml
        @info "key: $(key); key-type: $(typeof(key)); value-type: $(typeof(value))"
        if key == "openapi" api.openapi = value end
        if key == "info" parse!(api.info, value) end
        if key == "jsonSchemaDialect" api.jsonSchemaDialect = value end
        if key == "servers" parse!(api.servers, value) end
        if key == "paths"
            for (k,v) in value
                pi = PathItem()
                parse!(pi, v)
                api.paths[k] = pi
            end
        end
        if key == "webhooks"
            for (k,v) in value
                pi = PathItem()
                parse!(pi, v)
                api.webhooks[k] = pi
            end
        end
        if key == "components" parse!(api.components, value) end
        if key == "security" parse!(api.security, value) end
        if key == "tags" parse!(api.tags, value) end
        if key == "externalDocs" parse!(api.externalDocs, value) end
        if key == "x-tagGroups" parse!(api.tagGroups, value) end
    end

    api
end