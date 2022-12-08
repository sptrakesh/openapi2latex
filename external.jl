mutable struct ExternalDocumentation <: Comparable
    description::String
    url::URI
end

ExternalDocumentation() = ExternalDocumentation("", URI())