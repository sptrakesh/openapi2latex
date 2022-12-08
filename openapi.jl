import YAML: YAMLDocIterator, iterate, load_all_file
import URIs: uristring

mutable struct Tag
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

mutable struct Components
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

mutable struct TagGroup
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

mutable struct OpenAPI
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

function collect_tags!(o::OpenAPI)
    @debug "Collecting tags not included in global tags list"
    d = Dict{String,Tag}()
    for t in o.tags
        if !haskey(d, t.name) d[t.name] = t end
    end

    for (p, pi) in o.paths
        for t in pi.get.tags
            if !haskey(d, t) d[t] = Tag(t, "", ExternalDocumentation()); push!(o.tags, d[t]) end
        end
        for t in pi.put.tags
            if !haskey(d, t) d[t] = Tag(t, "", ExternalDocumentation()); push!(o.tags, d[t]) end
        end
        for t in pi.post.tags
            if !haskey(d, t) d[t] = Tag(t, "", ExternalDocumentation()); push!(o.tags, d[t]) end
        end
        for t in pi.delete.tags
            if !haskey(d, t) d[t] = Tag(t, "", ExternalDocumentation()); push!(o.tags, d[t]) end
        end
        for t in pi.options.tags
            if !haskey(d, t) d[t] = Tag(t, "", ExternalDocumentation()); push!(o.tags, d[t]) end
        end
        for t in pi.head.tags
            if !haskey(d, t) d[t] = Tag(t, "", ExternalDocumentation()); push!(o.tags, d[t]) end
        end
        for t in pi.patch.tags
            if !haskey(d, t) d[t] = Tag(t, "", ExternalDocumentation()); push!(o.tags, d[t]) end
        end
        for t in pi.trace.tags
            if !haskey(d, t) d[t] = Tag(t, "", ExternalDocumentation()); push!(o.tags, d[t]) end
        end
    end
end

function reference!(m::Any, path::String, key::String)
    if isempty(m.ref) return end
    if startswith(m.ref, "#") return end # TODO: populate from local reference

    r = findfirst("#", m.ref)
    fn = r === nothing ? "$(dirname(path))/$(m.ref)" : "$(dirname(path))/$(SubString(m.ref, 1, r[1] - 1))"
    if !isfile(fn) @warn "File $fn exists: $(isfile(fn)) does not exist!"; return end

    rpath = r === nothing ? "" : "$(SubString(m.ref, r[1]+2))"
    parts = split(rpath, "/")
    it = load_all_file(fn; dicttype=Dict{Any,Any})
    (yaml, state) = iterate(it)

    if r === nothing
        @info "Empty reference path for $key with $(m.ref)"
        parse!(m, yaml)
        return
    end

    for (key, value) in yaml
        if key == rpath
            @info "Parsing reference from $(m.ref) for path: $rpath"
            parse!(m, value)
        elseif length(parts) > 1 && key == parts[1] && value isa Dict{Any,Any}
            for (k,v) in value
                if k == parts[2]
                    @info "Parsing reference from $(m.ref) for path: $rpath"
                    parse!(m, v)
                end
            end
        end
    end
end

function collect_paths!(o::OpenAPI, path::String)
    @debug "Collecting path references"
    for (p, pi) in o.paths reference!(pi, path, p) end
end

function collect_tag_paths(o::OpenAPI)::Dict{String,Vector{PathItem}}
    @debug "Collecting PathInfo items for tags"
    ret = Dict{String,Vector{PathItem}}()

    function contains(v::Vector{PathItem}, pi::PathItem)::Bool
        for p in v
            if p == pi return true end
        end

        false
    end

    function add(o::Operation, pi::PathItem)
        for t in o.tags
            if haskey(ret, t)
                if !contains(ret[t], pi) push!(ret[t], pi) end
            else
                ret[t] = [pi]
            end
        end
    end

    for (p,pi) in o.paths
        add(pi.get, pi)
        add(pi.put, pi)
        add(pi.post, pi)
        add(pi.delete, pi)
        add(pi.options, pi)
        add(pi.head, pi)
        add(pi.patch, pi)
        add(pi.trace, pi)
    end

    ret
end

function collect_schemas!(o::OpenAPI, path::String)
    refs = Dict{String,Schema}()

    @debug "Populating schemas from components"
    for (key,s) in o.components.schemas
        if !isempty(s.ref)
            if haskey(refs, s.ref)
                saved = refs[s.ref]
                s.discriminator = saved.discriminator
                s.xml = saved.xml
                s.externalDocs = saved.externalDocs
                s.type = saved.type
                s.summary = saved.summary
                s.description = saved.description
                s.required = saved.required
                s.properties = saved.properties
            else
                reference!(s, path, key)
                refs[s.ref] = s
            end
        end
    end

    function retrieve(o::Operation, key::String)
        if !isempty(o.requestBody.ref)
            if haskey(refs, o.requestBody.ref)
                rb = refs[o.requestBody.ref]
                o.requestBody.description = rb.description
                o.requestBody.content = rb.content
                o.requestBody.required = rb.required
            else
                reference!(o.requestBody, path, key)
                refs[o.requestBody.ref] = o.requestBody
            end
        end

        for (k,c) in o.requestBody.content
            if isempty(c.schema.ref) continue end
            if haskey(refs, c.schema.ref)
                saved = refs[c.schema.ref]
                c.schema.discriminator = saved.discriminator
                c.schema.xml = saved.xml
                c.schema.externalDocs = saved.externalDocs
                c.schema.type = saved.type
                c.schema.summary = saved.summary
                c.schema.description = saved.description
                c.schema.required = saved.required
                c.schema.properties = saved.properties
            else
                reference!(c.schema, path, k)
                for (pk, p) in c.schema.properties
                    reference!(p, path, pk)
                end
                refs[c.schema.ref] = c.schema
            end
        end
    end

    @debug "Populating schemas from paths"
    for (key, p) in o.paths
        retrieve(p.get, key)
        retrieve(p.put, key)
        retrieve(p.post, key)
        retrieve(p.delete, key)
        retrieve(p.options, key)
        retrieve(p.head, key)
        retrieve(p.patch, key)
        retrieve(p.trace, key)
    end

    @debug "Collecting schemas from api and components"
    models = Dict{String,Schema}()
    for (key,s) in o.components.schemas
        models[key] = s
    end

    function add(o::Operation)
        for (key,value) in o.requestBody.content
            if isempty(value.schema.ref) continue end
            r = findfirst("#", value.schema.ref)
            fn = r === nothing ? "$(dirname(path))/$(value.schema.ref)" : "$(dirname(path))/$(SubString(value.schema.ref, 1, r[1] - 1))"
            rpath = r === nothing ? "" : "$(SubString(value.schema.ref, r[1]+2))"
            parts = split(rpath, "/")
            if isempty(parts) continue end
            models[last(parts)] = value.schema
        end
    end

    for (key,p) in o.paths
        add(p.get)
        add(p.put)
        add(p.post)
        add(p.delete)
        add(p.options)
        add(p.head)
        add(p.patch)
        add(p.trace)
    end

    models
end