__precompile__(true)

module model

import URIs: URI

abstract type CircularReference end

include("example.jl")
include("external.jl")
include("schema.jl")
include("security.jl")
include("server.jl")

include("header.jl")
include("info.jl")
include("mediatype.jl")
include("parameter.jl")
include("path.jl")

include("convert.jl")
include("openapi.jl")
include("output.jl")

end