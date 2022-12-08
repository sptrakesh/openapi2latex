mutable struct Contact <: Comparable
    name::String
    url::URI
    email::String
end

Contact() = Contact("", URI(), "")

function parse!(c::Contact, data::Dict{Any,Any})
    for (key, value) in data
        if key == "name" c.name = value end
        if key == "url" c.url = URI(value) end
        if key == "email" c.email = value end
    end
end

mutable struct License <: Comparable
    name::String
    identifier::String
    url::URI
end

License() = License("", "", URI())

function parse!(l::License, data::Dict{Any,Any})
    for (key, value) in data
        if key == "name" l.name = value end
        if key == "identifier" l.identifier = value end
        if key == "url" l.url = URI(value) end
    end
end

mutable struct Info <: Comparable
    title::String
    summary::String
    description::String
    termsOfService::URI
    contact::Contact
    license::License
    version::String
end

Info() = Info("", "", "", URI(), Contact(), License(), "")

function parse!(info::Info, data::Dict{Any,Any})
    for (key, value) in data
        if key == "title" info.title = value end
        if key == "summary" info.summary = value end
        if key == "description" info.description = convert(value) end
        if key == "termsOfService" info.termsOfService = URI(value) end
        if key == "contact" parse!(info.contact, value) end
        if key == "license" parse!(info.license, value) end
        if key == "version" info.version = value end
    end
end