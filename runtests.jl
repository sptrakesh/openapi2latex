using Test

include("model.jl")

@testset "Conversion tests" begin
    @testset "Style conversion tests" begin
        @testset "Bold with asterisks" begin
            line = "Micro-services for **ReelSense** data interactions."
            delim = "**"
            out = model.convert_style(line, delim, "\\textbf")
            @test "Micro-services for \\textbf{ReelSense} data interactions." == out
        end

        @testset "Bold with underscore" begin
            line = "Micro-services for __ReelSense__ data interactions."
            delim = "__"
            out = model.convert_style(line, delim, "\\textbf")
            @test "Micro-services for \\textbf{ReelSense} data interactions." == out
        end
    end
end