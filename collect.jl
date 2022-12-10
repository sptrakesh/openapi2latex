function collect_tags!(o::OpenAPI)
    @info "Collecting tags not included in global tags list"
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

import Base.Filesystem: joinpath
function follow_reference!(m::Comparable, path::String, key::String)
    if isempty(m.ref) return end
    if startswith(m.ref, "#") return end # TODO: populate from local reference

    r = findfirst("#", m.ref)
    fn = r === nothing ? joinpath(dirname(path), m.ref) : joinpath(dirname(path), SubString(m.ref, 1, r[1] - 1))
    if !isfile(fn) @warn "File $fn does not exist!"; return end

    m.referenceURI = URI(fn)
    @debug "Set referenceURI for $(typeof(m)) to $fn"

    rpath = r === nothing ? "" : "$(SubString(m.ref, r[1]+2))"
    parts = split(rpath, "/")
    it = load_all_file(fn; dicttype=OrderedDict{Any,Any})
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
        elseif length(parts) > 1 && key == parts[1] && value isa OrderedDict{Any,Any}
            for (k,v) in value
                if k == parts[2]
                    @info "Parsing reference from $(m.ref) for path: $rpath"
                    parse!(m, v)
                end
            end
        end
    end
end

function collect_parameters!(o::Operation, d::OrderedDict{String,Parameter}, path::URI)
    if isempty(o.parameters) return end
    for p in o.parameters
        if isempty(p.ref) continue end
        if startswith(p.ref, "#") continue end

        name = entity(p.ref)
        if haskey(d, name)
            saved = d[name]
            p.name = saved.name
            p.in = saved.in
            p.description = saved.description
            p.required = saved.required
            p.deprecated = saved.deprecated
            p.allowEmptyValue = saved.allowEmptyValue
            p.style = saved.style
            p.explode = saved.explode
            p.allowReserved = saved.allowReserved
            p.schema = saved.schema
            p.example = saved.example
            p.examples = saved.examples
            p.content = saved.content
            p.referenceURI = saved.referenceURI
        else
            follow_reference!(p, uristring(path), name)
            d[name] = p
        end
    end
end

function collect_paths!(o::OpenAPI, path::String)
    @info "Collecting path references"
    d = OrderedDict{String,Parameter}()

    for (p, pi) in o.paths
        follow_reference!(pi, path, p)
        collect_parameters!(pi.get, d, pi.referenceURI)
        collect_parameters!(pi.put, d, pi.referenceURI)
        collect_parameters!(pi.post, d, pi.referenceURI)
        collect_parameters!(pi.delete, d, pi.referenceURI)
        collect_parameters!(pi.options, d, pi.referenceURI)
        collect_parameters!(pi.head, d, pi.referenceURI)
        collect_parameters!(pi.patch, d, pi.referenceURI)
        collect_parameters!(pi.trace, d, pi.referenceURI)
    end
end

function collect_tag_paths(o::OpenAPI)::OrderedDict{String,Vector{Tuple{String,PathItem}}}
    @info "Collecting PathInfo items for tags"
    ret = OrderedDict{String,Vector{Tuple{String,PathItem}}}()

    function contains(v::Vector{Tuple{String,PathItem}}, pi::PathItem)::Bool
        for (x,p) in v
            if p == pi return true end
        end

        false
    end

    function add(o::Operation, pi::PathItem, p::String)
        tup = (p, pi)
        for t in o.tags
            if haskey(ret, t)
                if !contains(ret[t], pi) push!(ret[t], tup) end
            else
                ret[t] = [tup]
            end
        end
    end

    for (p,pi) in o.paths
        add(pi.get, pi, p)
        add(pi.put, pi, p)
        add(pi.post, pi, p)
        add(pi.delete, pi, p)
        add(pi.options, pi, p)
        add(pi.head, pi, p)
        add(pi.patch, pi, p)
        add(pi.trace, pi, p)
    end

    ret
end

function entity(key::String)::String
    r = findfirst("#", key)
    name = r === nothing ? "$(basename(key))" : "$(SubString(key, r[1]+2))"
    first(split(name, "."))
end

function collect_schemas!(o::OpenAPI, path::String)::OrderedDict{String,Schema}
    refs = OrderedDict{String,Schema}()

    @info "Populating schemas from components"
    for (key,s) in o.components.schemas
        if !isempty(s.ref)
            name = entity(s.ref)
            follow_reference!(s, path, key)
            refs[name] = s
        end
    end

    function retrieve(o::Operation, key::String, path::String)
        if !isempty(o.requestBody.ref)
            name = entity(o.requestBody.ref)
            if haskey(refs, name)
                rb = refs[name]
                o.requestBody.description = rb.description
                o.requestBody.content = rb.content
                o.requestBody.required = rb.required
                o.requestBody.referenceURI = rb.referenceURI
            else
                follow_reference!(o.requestBody, path, key)
                refs[name] = o.requestBody
            end
        end

        function process(s::Schema, key::String)
            if isempty(s.ref) return end
            name = entity(s.ref)
            if haskey(refs, name)
                saved = refs[name]
                s.discriminator = saved.discriminator
                s.xml = saved.xml
                s.externalDocs = saved.externalDocs
                s.type = saved.type
                s.summary = saved.summary
                s.description = saved.description
                s.required = saved.required
                s.properties = saved.properties
                s.referenceURI = saved.referenceURI
            else
                follow_reference!(s, path, key)
                uri = isempty(uristring(s.referenceURI)) ? path : uristring(s.referenceURI)
                for (pk, p) in s.properties
                    follow_reference!(p, uri, pk)
                end
                refs[name] = s
            end
        end

        for (k,c) in o.requestBody.content
            process(c.schema, k)
        end

        for p in o.parameters
            process(p.schema, p.name)
        end
    end

    @info "Populating schemas from paths"
    for (key, p) in o.paths
        if isempty(uristring(p.referenceURI)) @debug "referenceURI not set for path" end
        uri = isempty(uristring(p.referenceURI)) ? path : uristring(p.referenceURI)
        retrieve(p.get, key, uri)
        retrieve(p.put, key, uri)
        retrieve(p.post, key, uri)
        retrieve(p.delete, key, uri)
        retrieve(p.options, key, uri)
        retrieve(p.head, key, uri)
        retrieve(p.patch, key, uri)
        retrieve(p.trace, key, uri)
    end

    @info "Collecting schemas from api and components"

    function add(o::Operation)
        for (key,value) in o.requestBody.content
            if isempty(value.schema.ref) continue end
            name = entity(value.schema.ref)
            if haskey(refs, name)
                saved = refs[name]
                value.schema.discriminator = saved.discriminator
                value.schema.xml = saved.xml
                value.schema.externalDocs = saved.externalDocs
                value.schema.type = saved.type
                value.schema.summary = saved.summary
                value.schema.description = saved.description
                value.schema.required = saved.required
                value.schema.properties = saved.properties
                value.schema.referenceURI = saved.referenceURI
            else
                follow_reference!(value.schema, path, key)
                uri = isempty(uristring(value.schema.referenceURI)) ? path : uristring(value.schema.referenceURI)
                for (pk, p) in value.schema.properties
                    follow_reference!(p, uri, pk)
                end
                refs[name] = value.schema
            end
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

    @info "Collecting schema references from schema properties"
    for (key, schema) in refs
        for (k,prop) in schema.properties
            if isempty(prop.type) && !isempty(prop.ref)
                uri = isempty(uristring(prop.referenceURI)) ? uristring(schema.referenceURI) : uristring(prop.referenceURI)
                if isempty(uri) uri = path end
                follow_reference!(prop, uri, k)
            end
        end
    end

    refs
end
