_processed = Dict{String, OrderedDict{Any,Any}}()

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
function follow_reference!(m::Comparable, path::String, key::String)::OrderedDict{String,Schema}
    ret = OrderedDict{String,Schema}()

    if isempty(m.ref) return ret end

    r = findfirst("#", m.ref)
    if startswith(m.ref, "#")
        fn = path
    else
        fn = r === nothing ? joinpath(dirname(path), m.ref) : joinpath(dirname(path), SubString(m.ref, 1, r[1] - 1))
    end

    if !isfile(fn) @warn "File $fn does not exist!"; return end

    m.referenceURI = URI(fn)
    @debug "Set referenceURI for $(typeof(m)) to $fn"

    rpath = r === nothing ? "" : "$(SubString(m.ref, r[1]+2))"
    parts = split(rpath, "/")
    yaml = OrderedDict{Any,Any}()

    if haskey(_processed, fn)
        yaml = _processed[fn]
    else
        it = load_all_file(fn; dicttype=OrderedDict{Any,Any})
        (y, state) = iterate(it)
        _processed[fn] = y
        yaml = y
    end

    if r === nothing
        @info "Empty reference path for $key with $(m.ref) for $(typeof(m))"
        parse!(m, yaml)
        if m isa Schema
            ek = first(split(basename(m.ref), "."))
            @debug "Adding key: $ek to return dict"
            ret[ek] = m
        end
        return ret
    end

    d = yaml
    for i in 1:length(parts)
        if  haskey(d, parts[i])
            if i < length(parts) d = d[parts[i]]; continue end
            @info "Parsing reference $(m.ref) from path: $rpath in iteration $i for $(parts[i])"
            parse!(m, d[parts[i]])
        else
            @warn "Part $(parts[i]) in key $key does not exist in $fn"
            break
        end
    end

    function populate(schema::Schema)::Union{String,Nothing}
        r = findfirst("#", schema.ref)
        rpath = "$(SubString(schema.ref, r[1]+2))"
        @debug "Populating schema for $rpath"
        parts = split(rpath, "/")
        res::Union{String,Nothing} = nothing

        d = yaml
        for i in 1:length(parts)
            if haskey(d, parts[i])
                if i < length(parts) d = d[parts[i]]; continue end
                if haskey(d[parts[i]], "type")
                    if d[parts[i]]["type"] == "object"
                        s = Schema()
                        parse!(s, d[parts[i]])
                        s.referenceURI = URI(fn)
                        res = first(split(basename(fn), ".")) * "::" * parts[i]
                        @debug "Adding $res to return dictionary"
                        ret[res] = s
                    else
                        parse!(schema, d[parts[i]])
                        schema.referenceURI = URI(fn)
                    end
                else
                    parse!(schema, d[parts[i]])
                    schema.referenceURI = URI(fn)
                end
            else
                @warn "Part $(parts[i]) in key $k does not exist in $fn"
            end
        end

        if res isa String @debug "Populated schema with key $res" end
        res
    end

    if m isa Schema
        kn = startswith(m.ref, "#/") ? entity(fn * m.ref) : entity(m.ref)
        @debug "Adding $kn to return dictionary"
        ret[kn] = m

        for (k,schema) in m.properties
            sc = schema.items isa Schema ? schema.items : schema
            if isempty(sc.ref) continue end
            if startswith(sc.ref, "#/")
                res = populate(sc)
                if res isa String
                    if !haskey(ret, res)
                        @debug "Adding $res to return dictionary"
                        ret[res] = sc
                    end
                end
            else
                uri = isempty(uristring(sc.referenceURI)) ? uristring(sc.referenceURI) : uristring(schema.referenceURI)
                if isempty(uri) uri = isempty(uristring(m.referenceURI)) ? path : uristring(m.referenceURI) end
                @debug "Following external reference for $(sc.title) at $uri"
                follow_reference!(sc, uri, k)
            end
        end

        for schema in m.allOf
            if isempty(schema.ref) continue end
            if startswith(schema.ref, "#/")
                res = populate(schema)
                if res isa String
                    for (k,v) in ret[res].properties m.properties[k] = v end
                end
            else
                uri = isempty(uristring(schema.referenceURI)) ? uristring(m.referenceURI) : uristring(schema.referenceURI)
                if isempty(uri) uri = path end
                follow_reference!(schema, uri, "allOf")
            end
        end

        function composite(schema::Schema, key::String)
            if isempty(schema.ref) return end
            if startswith(schema.ref, "#/")
                res = populate(schema)
                if res isa String
                    if !haskey(ret, res)
                        @debug "Adding $res to return dictionary"
                        ret[res] = schema
                    end
                end
            else
                uri = isempty(uristring(schema.referenceURI)) ? uristring(m.referenceURI) : uristring(schema.referenceURI)
                if isempty(uri) uri = path end
                follow_reference!(schema, uri, key)
            end
        end

        for schema in m.oneOf composite(schema, "oneOf") end

        for schema in m.anyOf composite(schema, "anyOf") end

        if m.items isa Schema
            @debug "Populating local references for array items in $(m.title)"
            if startswith(m.items.ref, "#/") populate(m.items) end
        end
    end

    ret
end

function collect_parameters!(o::Operation, d::OrderedDict{String,Parameter}, path::URI)::OrderedDict{String,Schema}
    ret = OrderedDict{String,Schema}()
    if isempty(o.parameters) return ret end
    for p in o.parameters
        if isempty(p.ref) continue end
        if startswith(p.ref, "#")
            @warn "Parameter with local reference $(p.ref) at path: $(uristring(path))"
            continue
        end

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
            res = follow_reference!(p, uristring(path), name)
            for (k,v) in res if !haskey(ret, k) ret[k] = v end end
            d[name] = p
        end
    end

    ret
end

function collect_responses!(o::Operation, r::Dict{String,Response}, s::Dict{String,Schema}, path::URI)::OrderedDict{String,Schema}
    ret = OrderedDict{String,Schema}()

    function follow(sc::Schema, key::String)
        if isempty(sc.ref) return end
        name = entity(sc.ref)
        if haskey(s, name)
            sc.referenceURI = s[name].referenceURI
        else
            res = follow_reference!(sc, uristring(path), key)
            for (k,v) in res
                if !haskey(ret, k)
                    @debug "Adding $k to return dictionary"
                    ret[k] = v
                end
            end

            ref = startswith(sc.ref, "#") ? uristring(sc.referenceURI) * sc.ref : sc.ref
            name = entity(ref)
            @debug "Adding $name to input dictionary from $path"
            s[name] = sc
        end
    end

    for (key,value) in o.responses
        if !isempty(value.ref)
            name = entity(value.ref)
            if haskey(r, name)
                saved = r[name]
                if isempty(value.description) value.description = saved.description end
                value.headers = saved.headers
                value.content = saved.content
                value.links = saved.links
                value.referenceURI = saved.referenceURI
            else
                res = follow_reference!(value, uristring(path), key)
                for (k,v) in res
                    if !haskey(ret, k)
                        @debug "Adding $k to return dictionary"
                        ret[k] = v
                    end
                end
                r[name] = value
            end
        end

        for (k,v) in value.content
            if !(v.schema isa Schema) continue end
            if !isempty(v.schema.ref)
                name = entity(v.schema.ref)
                if haskey(s, name)
                    v.schema.referenceURI = s[name].referenceURI
                else
                    res = follow_reference!(v.schema, uristring(path), k)
                    for (k,v) in res
                        if !haskey(ret, k)
                            @debug "Adding $k to return dictionary"
                            ret[k] = v
                        end
                    end
                    s[name] = v.schema
                end
            end

            for sc in v.schema.oneOf follow(sc, k) end
            for sc in v.schema.anyOf follow(sc, k) end
            for sc in v.schema.allOf follow(sc, k) end
        end
    end

    ret
end

function collect_paths!(o::OpenAPI, path::String)::Tuple{Dict{String,Schema},Dict{String,Response}}
    @info "Collecting path references"
    d = OrderedDict{String,Parameter}()
    r = Dict{String,Response}()
    s = Dict{String,Schema}()

    function add(res::OrderedDict{String,Schema})
        for (k,v) in res
            if !haskey(s, k)
                @debug "Adding $k to return dictionary"
                s[k] = v
            end
        end
    end

    for (p, pi) in o.paths
        res = follow_reference!(pi, path, p)
        for (k,v) in res
            if !haskey(s, k)
                @debug "Adding $k to return dictionary"
                s[k] = v
            end
        end
        collect_parameters!(pi.get, d, pi.referenceURI)
        collect_parameters!(pi.put, d, pi.referenceURI)
        collect_parameters!(pi.post, d, pi.referenceURI)
        collect_parameters!(pi.delete, d, pi.referenceURI)
        collect_parameters!(pi.options, d, pi.referenceURI)
        collect_parameters!(pi.head, d, pi.referenceURI)
        collect_parameters!(pi.patch, d, pi.referenceURI)
        collect_parameters!(pi.trace, d, pi.referenceURI)

        res = collect_responses!(pi.get, r, s, pi.referenceURI)
        add(res)
        res = collect_responses!(pi.put, r, s, pi.referenceURI)
        add(res)
        res = collect_responses!(pi.post, r, s, pi.referenceURI)
        add(res)
        res = collect_responses!(pi.delete, r, s, pi.referenceURI)
        add(res)
        res = collect_responses!(pi.options, r, s, pi.referenceURI)
        add(res)
        res = collect_responses!(pi.head, r, s, pi.referenceURI)
        add(res)
        res = collect_responses!(pi.patch, r, s, pi.referenceURI)
        add(res)
        res = collect_responses!(pi.trace, r, s, pi.referenceURI)
        add(res)
    end

    (s, r)
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
    if r === nothing
        first(split(basename(key), "."))
    else
        "$(first(split(basename(SubString(key, 1, r[1]-1)), ".")))::$(last(split(SubString(key, r[1]+2), "/")))"
    end
end

function collect_schemas!(o::OpenAPI, path::String, rschemas::Dict{String,Schema})::OrderedDict{String,Schema}
    refs = OrderedDict{String,Schema}()

    function add(res::OrderedDict{String,Schema})
        for (k,v) in res
            if !haskey(refs, k)
                @debug "Adding collected key $k"
                refs[k] = v
            end
        end
    end

    @info "Populating schemas from components"
    for (key,s) in o.components.schemas
        if !isempty(s.ref)
            name = entity(s.ref)

            if !haskey(refs, name)
                res = follow_reference!(s, path, key)
                add(res)
                @debug "Adding $name to refs"
                refs[name] = s
            end
        else refs["::$key"] = s
        end
    end

    for (key,value) in rschemas
        if !haskey(refs, key)
            @debug "Adding $key to refs"
            refs[key] = value
        end
    end

    function retrieve(o::Operation, key::String, path::String)
        function process(s::Schema, key::String)
            if isempty(s.ref) return end
            name = entity(s.ref)
            if haskey(refs, name)
                s.referenceURI = refs[name].referenceURI
            else
                res = follow_reference!(s, path, key)
                uri = isempty(uristring(s.referenceURI)) ? path : uristring(s.referenceURI)
                for (pk, p) in s.properties
                    res = follow_reference!(p, uri, pk)
                    add(res)
                end
                refs[name] = s
            end
        end

        if o.requestBody isa RequestBody
            if !isempty(o.requestBody.ref)
                name = entity(o.requestBody.ref)
                if haskey(refs, name)
                    o.requestBody.referenceURI = refs[name].referenceURI
                else
                    res = follow_reference!(o.requestBody, path, key)
                    add(res)
                    refs[name] = o.requestBody
                end
            end

            for (k,c) in o.requestBody.content process(c.schema, k) end
        end

        for p in o.parameters process(p.schema, p.name) end
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
        if o.requestBody isa Nothing return end
        for (key,value) in o.requestBody.content
            if isempty(value.schema.ref) continue end
            name = entity(value.schema.ref)
            if haskey(refs, name)
                value.schema.referenceURI = refs[name].referenceURI
            else
                res = follow_reference!(value.schema, path, key)
                add(res)
                uri = isempty(uristring(value.schema.referenceURI)) ? path : uristring(value.schema.referenceURI)
                for (pk, p) in value.schema.properties
                    res = follow_reference!(p, uri, pk)
                    add(res)
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

    @info "Collection schema references from allOf"
    for (key,schema) in refs
        if isempty(schema.allOf) continue end
        for item in schema.allOf
            if !isempty(item.ref) && !startswith(item.ref, "#/")
                uri = isempty(uristring(item.referenceURI)) ? uristring(schema.referenceURI) : uristring(item.referenceURI)
                if isempty(uri) uri = path end
                res = follow_reference!(item, uri, key)
                add(res)
            end
        end
    end

    @info "Collecting schema references from schema properties"
    for (key, schema) in refs
        for (k,prop) in schema.properties
            if !isempty(prop.ref)
                uri = isempty(uristring(prop.referenceURI)) ? uristring(schema.referenceURI) : uristring(prop.referenceURI)
                if isempty(uri) uri = path end
                res = follow_reference!(prop, uri, k)
                add(res)
            end
        end
    end

    @info "Collecting schema references from schema items"
    for (key,value) in refs
        if value.items === nothing continue end
        if isempty(value.items.ref) continue end
        uri = isempty(uristring(value.items.referenceURI)) ? uristring(value.referenceURI) : uristring(value.items.referenceURI)
        if isempty(uri) uri = path end
        res = follow_reference!(prop, uri, key)
        add(res)
    end

    out = OrderedDict{String,Schema}()
    #for (k,s) in o.components.schemas out[k] = s end

    k = Vector{String}()
    for key in keys(refs) push!(k, key) end
    for key in sort(k)
        if haskey(out, key) continue end
        out[key] = refs[key]
    end

    out
end
