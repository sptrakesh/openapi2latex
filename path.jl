mutable struct RequestBody <: Comparable
    ref::String
    description::String
    content::OrderedDict{String,MediaType}
    required::Bool
    referenceURI::URI # To track external references.
end

RequestBody() = RequestBody("", "", OrderedDict{String,MediaType}(), false, URI())

function parse!(r::RequestBody, data::OrderedDict{Any,Any})
    for (key,value) in data
        if key == "\$ref" p.ref = value end
        if key == "desription" r.description = value end
        if key == "content" parse!(r.content, value) end
        if key == "required" r.required = value end
    end
end

function parse!(r::RequestBody, data::OrderedDict{Any,Any})
    for (key,value) in data
        if key == "\$ref" p.ref = value end
        if key == "desription" r.description = value end
        if key == "content" parse!(r.content, value) end
        if key == "required" r.required = value end
    end
end

mutable struct Link <: Comparable
    operationRef::String
    operationId::String
    parameters::OrderedDict{String,Any}
    requestBody::Any
    description::String
    server::Server
end

Link() = Link("", "", OrderedDict{String,Any}(), Nothing, "", Server())

function parse!(l::Link, data::OrderedDict{Any,Any})
    for (key,value) in data
    if key == "operationRef" l.operationRef = value end
    if key == "operationId" l.operationId = value end
    if key == "parameters" l.parameters = value end
    if key == "requestBody" l.requestBody = value end
    if key == "description" l.description = value end
    if key == "server" parse!(l.server, value) end
    end
end

function parse!(l::OrderedDict{String,Link}, data::OrderedDict{Any,Any})
    for (key,value) in data
        lnk = Link()
        parse!(lnk, value)
        l[key] = lnk
    end
end

mutable struct Response <: Comparable
    ref::String
    description::String
    headers::OrderedDict{String,Header}
    content::OrderedDict{String,MediaType}
    links::OrderedDict{String,Link}
    referenceURI::URI # To track external references.
end

Response() = Response("", "", OrderedDict{String,Header}(), OrderedDict{String,MediaType}(), OrderedDict{String,Link}(), URI())

function parse!(r::Response, data::OrderedDict{Any,Any})
    for (key,value) in data
        if key == "\$ref" r.ref = value end
        if key == "description" r.description = value end
        if key == "headers" parse!(r.headers, value) end
        if key == "content" parse!(r.content, value) end
        if key == "link" parse!(r.link, value) end
    end
end

function parse!(r::OrderedDict{String,Response}, data::OrderedDict{Any,Any})
    for (key,value) in data
        res = Response()
        parse!(res, value)
        r["$key"] = res
    end
end

mutable struct Operation{PI<:CircularReference} <: Comparable
    tags::Vector{String}
    summary::String
    description::String
    externalDocs::Union{ExternalDocumentation,Nothing}
    operationId::String
    parameters::Vector{Parameter}
    requestBody::Union{RequestBody,Nothing}
    responses::OrderedDict{String,Response}
    callbacks::OrderedDict{String,PI}
    deprecated::Bool
    security::Union{Vector{SecurityRequirement},Nothing}
    servers::Vector{Server}
    codeSamples::Vector{OrderedDict{String,Any}}
end

Operation() = Operation(Vector{String}(), "", "", nothing, "", Vector{Parameter}(),
    nothing, OrderedDict{String,Response}(), OrderedDict{String,PathItem}(), false,
    nothing, Vector{Server}(), Vector{OrderedDict{String,Any}}())

function parse!(o::Operation, data::OrderedDict{Any,Any})
    for (key, value) in data
        if key == "tags"
            for v in value push!(o.tags, v) end
        end
        if key == "summary" o.summary = value end
        if key == "description" o.description = value end
        if key == "externalDocs" o.externalDocs = ExternalDocumentation(); parse!(o.externalDocs, value) end
        if key == "operationId" o.operationId = value end
        if key == "parameters" parse!(o.parameters, value) end
        if key == "requestBody" o.requestBody = RequestBody(); parse!(o.requestBody, value) end
        if key == "responses" parse!(o.responses, value) end
        if key == "callbacks" parse!(o.callbacks, value) end
        if key == "deprecated" o.deprecated = value end
        if key == "security"
            o.security = Vector{SecurityRequirement}()
            parse!(o.security, value)
            @info "Parsed security for operation $(o.operationId); $(json(o.security))"
        end
        if key == "servers" parse!(o.servers, value) end
        if key == "x-codeSamples"
            for cs in value
                d = OrderedDict{String,Any}()
                for (k,v) in cs d["$k"] = v end
                push!(o.codeSamples, d)
            end
        end
    end
end

mutable struct PathItem <: CircularReference
    ref::String
    summary::String
    description::String
    get::Operation
    put::Operation
    post::Operation
    delete::Operation
    options::Operation
    head::Operation
    patch::Operation
    trace::Operation
    servers::Vector{Server}
    parameters::Vector{Parameter}
    referenceURI::URI # To track external references.
end

PathItem() = PathItem("", "", "", Operation(), Operation(), Operation(), Operation(), Operation(),
    Operation(), Operation(), Operation(), Vector{Server}(), Vector{Parameter}(), URI())

function parse!(p::PathItem, data::OrderedDict{Any,Any})
    for (key, value) in data
        if key == "\$ref" p.ref = value end
        if key == "summary" p.summary = value end
        if key == "description" p.description = value end
        if key == "get" parse!(p.get, value) end
        if key == "put" parse!(p.put, value) end
        if key == "post" parse!(p.post, value) end
        if key == "delete" parse!(p.delete, value) end
        if key == "options" parse!(p.options, value) end
        if key == "head" parse!(p.head, value) end
        if key == "patch" parse!(p.patch, value) end
        if key == "trace" parse!(p.trace, value) end
        if key == "servers" parse!(p.servers, value) end
        if key == "parameters" parse!(p.parameters, value) end
    end

    if !isempty(p.servers) setservers!(p, p.servers) end
end

function setservers!(p::PathItem, servers::Vector{Server})
    p.servers = servers

    if !isempty(p.servers)
        if isempty(p.get.servers) p.get.servers = p.servers end
        if isempty(p.put.servers) p.put.servers = p.servers end
        if isempty(p.post.servers) p.post.servers = p.servers end
        if isempty(p.delete.servers) p.delete.servers = p.servers end
        if isempty(p.options.servers) p.options.servers = p.servers end
        if isempty(p.head.servers) p.head.servers = p.servers end
        if isempty(p.patch.servers) p.patch.servers = p.servers end
        if isempty(p.trace.servers) p.trace.servers = p.servers end
    end
end

function setsecurity!(p::PathItem, security::Vector{SecurityRequirement})
    if p.get.security === nothing p.get.security = security
    else @debug "Operation $(p.get.operationId) has custom security requirement" end

    if p.put.security === nothing p.put.security = security
    else @debug "Operation $(p.put.operationId) has custom security requirement" end

    if p.post.security === nothing p.post.security = security
    else @debug "Operation $(p.post.operationId) has custom security requirement" end

    if p.delete.security === nothing p.delete.security = security
    else @debug "Operation $(p.delete.operationId) has custom security requirement" end

    if p.options.security === nothing p.options.security = security
    else @debug "Operation $(p.options.operationId) has custom security requirement" end

    if p.head.security === nothing p.head.security = security
    else @debug "Operation $(p.head.operationId) has custom security requirement" end

    if p.patch.security === nothing p.patch.security = security
    else @debug "Operation $(p.patch.operationId) has custom security requirement" end

    if p.trace.security === nothing p.trace.security = security
    else @debug "Operation $(p.trace.operationId) has custom security requirement" end
end