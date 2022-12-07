import ArgParse: ArgParseSettings, @add_arg_table, parse_args
import YAML: load_all_file
using MiniLoggers

include("model.jl")

function cmd_options()
    s = ArgParseSettings()
    @add_arg_table s begin
        "--input", "-i"
            help = "The input OpenAPI YAML file"
            arg_type = String
            required = true
        "--output", "-o"
            help = "The fully qualified filename for the output LaTeX file."
            arg_type = String
            required = true
        "--author", "-a"
            help = "The author of the document."
            arg_type = String
            required = false
            default = "OpenAPI2LaTeX Generator"
        "--debug", "-d"
        help = "Enable debug log level"
        action = :store_true
    end
    parse_args(ARGS, s)
end

function main()
    args = cmd_options()
    if args["debug"]
        MiniLogger(minlevel = MiniLoggers.Debug,
               format = "{[{timestamp}] [{level}] [:func}{{module}@{basename}:{line:cyan}:light_green}]: {message}") |> global_logger
    else
        MiniLogger(minlevel = MiniLoggers.Info,
               format = "{[{timestamp}] [{level}] [:func}{{module}@{basename}:{line:cyan}:light_green}]: {message}") |> global_logger
    end
    spec = load_all_file(args["input"]; dicttype=Dict{Any,Any})
    api = model.parse(spec)
    model.generate(api, args["output"], args["author"], args["input"])
end

main()