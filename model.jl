__precompile__(true)

module model

import OrderedCollections: OrderedDict
import URIs: URI

abstract type Comparable end
import Base.==
==(l::T, r::T) where T <: Comparable =
    getfield.(Ref(l),fieldnames(T)) == getfield.(Ref(r),fieldnames(T))
function copyfrom(l::T, r::T) where T <: Comparable
    for f in fieldnames(T) getfield.(Ref(l), f) = getfield.(Ref(r), f) end
end

abstract type CircularReference <: Comparable end

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
include("collect.jl")
include("output.jl")

end