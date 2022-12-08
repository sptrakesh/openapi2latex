mutable struct RequestBody <: Comparable
    ref::String
    description::String
    content::Dict{String,MediaType}
    required::Bool
end

RequestBody() = RequestBody("", "", Dict{String,MediaType}(), false)

function parse!(r::RequestBody, data::Dict{Any,Any})
    for (key,value) in data
        if key == "\$ref" p.ref = value end
        if key == "desription" r.description = convert(value) end
        if key == "content" parse!(r.content, value) end
        if key == "required" r.required = value end
    end
end

function parse!(r::RequestBody, data::Dict{Any,Any})
    for (key,value) in data
        if key == "\$ref" p.ref = value end
        if key == "desription" r.description = convert(value) end
        if key == "content" parse!(r.content, value) end
        if key == "required" r.required = value end
    end
end

mutable struct Link <: Comparable
    operationRef::String
    operationId::String
    parameters::Dict{String,Any}
    requestBody::Any
    description::String
    server::Server
end

Link() = Link("", "", Dict{String,Any}(), Nothing, "", Server())

function parse!(l::Link, data::Dict{Any,Any})
    for (key,value) in data
    if key == "operationRef" l.operationRef = value end
    if key == "operationId" l.operationId = value end
    if key == "parameters" l.parameters = value end
    if key == "requestBody" l.requestBody = value end
    if key == "description" l.description = value end
    if key == "server" parse!(l.server, value) end
    end
end

function parse!(l::Dict{String,Link}, data::Dict{Any,Any})
    for (key,value) in data
        lnk = Link()
        parse!(lnk, value)
        l[key] = lnk
    end
end

mutable struct Response <: Comparable
    description::String
    headers::Dict{String,Header}
    content::Dict{String,MediaType}
    links::Dict{String,Link}
end

Response() = Response("", Dict{String,Header}(), Dict{String,MediaType}(), Dict{String,Link}())

function parse!(r::Response, data::Dict{Any,Any})
    for (key,value) in data
        if key == "description" r.description = value end
        if key == "headers" parse!(r.headers, value) end
        if key == "content" parse!(r.content, value) end
        if key == "link" parse!(r.link, value) end
    end
end

function parse!(r::Dict{String,Response}, data::Dict{Any,Any})
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
    externalDocs::ExternalDocumentation
    operationId::String
    parameters::Vector{Parameter}
    requestBody::RequestBody
    responses::Dict{String,Response}
    callbacks::Dict{String,PI}
    deprecated::Bool
    security::Vector{SecurityRequirement}
    servers::Vector{Server}
end

Operation() = Operation(Vector{String}(), "", "", ExternalDocumentation(), "", Vector{Parameter}(),
    RequestBody(), Dict{String,Response}(), Dict{String,PathItem}(), false,
    Vector{SecurityRequirement}(), Vector{Server}())

function parse!(o::Operation, data::Dict{Any,Any})
    for (key, value) in data
        if key == "tags"
            for v in value push!(o.tags, v) end
        end
        if key == "summary" o.summary = value end
        if key == "description" o.description = convert(value) end
        if key == "externalDocs" parse!(o.externalDocs, value) end
        if key == "operationId" o.operationId = value end
        if key == "parameters" parse!(o.parameters, value) end
        if key == "requestBody" parse!(o.requestBody, value) end
        if key == "responses" parse!(o.responses, value) end
        if key == "callbacks" parse!(o.callbacks, value) end
        if key == "deprecated" o.deprecated = value end
        if key == "security" parse!(o.security, value) end
        if key == "servers" parse!(o.servers, value) end
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
    codeSamples::Vector{Dict{String,Any}}
end

PathItem() = PathItem("", "", "", Operation(), Operation(), Operation(), Operation(), Operation(),
    Operation(), Operation(), Operation(), Vector{Server}(), Vector{Parameter}(), Vector{Dict{String,Any}}())

function parse!(p::PathItem, data::Dict{Any,Any})
    for (key, value) in data
        if key == "\$ref" p.ref = value end
        if key == "summary" p.summary = value end
        if key == "description" p.description = convert(value) end
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
        if key == "x-codeSamples"
            @info "x-codeSamples with type $(typeof(value))"
            for cs in value
                d = Dict{String,Any}()
                for (k,v) in cs d["$k"] = v end
                push!(p.codeSamples, d)
            end
        end
    end
end
