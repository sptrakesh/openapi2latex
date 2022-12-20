mutable struct ServerVariable <: Comparable
    enum::Vector{String}
    default::String
    description::String
end

ServerVariable() = ServerVariable(Vector{String}(), "", "")

function parse!(s::ServerVariable, data::OrderedDict{Any,Any})
    for (key, value) in data
        if key == "enum"
            for v in value push!(s.enum, v) end
        end
        if key == "default" s.default = value end
        if key == "description" s.description = value end
    end
end

mutable struct Server <: Comparable
    url::URI
    description::String
    variables::OrderedDict{String,ServerVariable}
end

Server() = Server(URI(), "", OrderedDict{String,ServerVariable}())

function parse!(s::Server, data::OrderedDict{Any,Any})
    for (key,value) in data
        if key == "url" s.url = URI(value) end
        if key == "description" s.description = value end
        if key == "variables"
            for (k,v) in value
                sv = ServerVariable()
                parse!(sv, v)
                s.variables[k] = sv
            end
        end
    end
end

function parse!(s::Vector{Server}, data::Vector{OrderedDict{Any,Any}})
    for d in data
        srv = Server()
        parse!(srv, d)
        push!(s, srv)
    end
end
