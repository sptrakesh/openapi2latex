mutable struct ExternalDocumentation
    description::String
    url::URI
end

ExternalDocumentation() = ExternalDocumentation("", URI())