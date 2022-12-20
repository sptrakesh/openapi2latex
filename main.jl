__precompile__(true)

import ArgParse: ArgParseSettings, @add_arg_table!, parse_args
import OrderedCollections: OrderedDict
import YAML: load_file
using MiniLoggers

include("model/model.jl")

function cmd_options()
    s = ArgParseSettings()
    @add_arg_table! s begin
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
        "--footer", "-f"
            help = "The right footer text for the document."
            arg_type = String
            required = false
            default = "Proprietary and Confidential"
        "--debug", "-d"
        help = "Enable debug log level"
        action = :store_true
    end
    parse_args(s)
end

function main(args)
    @time begin
        opts = cmd_options()
        if opts["debug"]
            MiniLogger(minlevel = MiniLoggers.Debug,
                   format = "{[{timestamp}] [{level}] [:func}{{module}@{basename}:{line:cyan}:light_green}]: {message}") |> global_logger
        else
            MiniLogger(minlevel = MiniLoggers.Info,
                   format = "{[{timestamp}] [{level}] [:func}{{module}@{basename}:{line:cyan}:light_green}]: {message}") |> global_logger
        end
        spec = load_file(opts["input"]; dicttype=OrderedDict{Any,Any})
        api = model.parse(spec)
        model.generate!(api, opts)
    end
end

main(ARGS)