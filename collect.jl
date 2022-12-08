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

function follow_reference!(m::Any, path::String, key::String)
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
    for (p, pi) in o.paths follow_reference!(pi, path, p) end
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

function collect_schemas!(o::OpenAPI, path::String)::OrderedDict{String,Schema}
    refs = OrderedDict{String,Schema}()

    function entity(key::String)::String
        r = findfirst("#", key)
        name = r === nothing ? "$(basename(key))" : "$(SubString(key, r[1]+2))"
        first(split(name, "."))
    end

    @debug "Populating schemas from components"
    for (key,s) in o.components.schemas
        if !isempty(s.ref)
            name = entity(s.ref)
            follow_reference!(s, path, key)
            refs[name] = s
        end
    end

    return refs

    function retrieve(o::Operation, key::String)
        if !isempty(o.requestBody.ref)
            if haskey(refs, o.requestBody.ref)
                rb = refs[o.requestBody.ref]
                o.requestBody.description = rb.description
                o.requestBody.content = rb.content
                o.requestBody.required = rb.required
            else
                follow_reference!(o.requestBody, path, key)
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
                follow_reference!(c.schema, path, k)
                for (pk, p) in c.schema.properties
                    follow_reference!(p, path, pk)
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
    models = OrderedDict{String,Schema}()
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
