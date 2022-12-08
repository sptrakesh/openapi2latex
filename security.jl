mutable struct SecurityRequirement <: Comparable
    name::Vector{String}
end

SecurityRequirement() = SecurityRequirement(Vector{String}())

function parse!(s::SecurityRequirement, data::OrderedDict{Any,Any})
    for (key, value) in data
        if key == "name"
            for v in value push!(s.name, v) end
        end
    end
end

function parse!(s::Vector{SecurityRequirement}, data::Vector{OrderedDict{Any,Any}})
    for d in data
        sr = SecurityRequirement()
        parse!(sr, d)
        push!(s, sr)
    end
end

mutable struct OAuthFlow <: Comparable
    authorizationUrl::URI
    tokenUrl::URI
    refreshUrl::URI
    scopes::OrderedDict{String,String}
end

OAuthFlow() = OAuthFlow(URI(), URI(), URI(), OrderedDict{String,String}())

function parse!(o::OAuthFlow, data::OrderedDict{Any,Any})
    for (key,value) in data
        if key == "authorizationUrl" o.authorizationUrl = URI(value) end
        if key == "tokenUrl" o.authorizationUrl = URI(value) end
        if key == "refreshUrl" o.refreshUrl = URI(value) end
        if key == "scopes"
             for (k,v) in value o.scopes[k] = v end
         end
    end
end

mutable struct OAuthFlows <: Comparable
    implicit::OAuthFlow
    password::OAuthFlow
    clientCredentials::OAuthFlow
    authorizationCode::OAuthFlow
end

OAuthFlows() = OAuthFlows(OAuthFlow(), OAuthFlow(), OAuthFlow(), OAuthFlow())

function parse!(o::OAuthFlows, data::OrderedDict{Any,Any})
    for (key,value) in data
        if key == "implicit" parse!(o.implicit, value) end
        if key == "password" parse!(o.password, value) end
        if key == "clientCredentials" parse!(o.clientCredentials, value) end
        if key == "authorizationCode" parse!(o.authorizationCode, value) end
    end
end

mutable struct SecurityScheme <: Comparable
    ref::String
    type::String
    description::String
    name::String
    in::String
    scheme::String
    bearerFormat::String
    flows::OAuthFlows
    openIdConnectUrl::URI
end

SecurityScheme() = SecurityScheme("", "", "", "", "", "", "", OAuthFlows(), URI())

function parse!(s::SecurityScheme, data::OrderedDict{Any,Any})
    for (key,value) in data
        if key == "\$ref" s.ref = value end
        if key == "type" s.type = value end
        if key == "description" s.description = value end
        if key == "name" s.name = value end
        if key == "in" s.in = value end
        if key == "scheme" s.scheme = value end
        if key == "bearerFormat" s.bearerFormat = value end
        if key == "flows" parse!(s.bearerFormat, value) end
        if key == "openIdConnectUrl" s.openIdConnectUrl = URI(value) end
    end
end

function parse!(s::Vector{SecurityRequirement}, data::Vector{OrderedDict{Any,Any}})
    for d in data
        sr = SecurityRequirement()
        parse!(sr, d)
        push!(s, sr)
    end
end