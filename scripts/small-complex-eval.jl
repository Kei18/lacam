# experimental scripts for small & complicated instances
import CSV
import Dates

function main()
    seeds = 5
    time_limit_sec = 10
    scens = [
        "connector",
        "corners",
        "loop-chain",
        "string",
        "tree",
        "tunnel"
    ]
    output_file = "build/result.txt"
    date_str = replace(string(Dates.now()), ":" => "-")
    root_dir = joinpath(@__DIR__, "..", "..", "data", "exp", date_str)
    !isdir(root_dir) && mkpath(root_dir)

    result = Vector{Any}()
    for scen in scens
        scen_file = "assets/scen/$(scen).scen"
        map_file = "assets/map/$(scen).map"
        N = length(readlines(scen_file))
        for seed in 1:seeds
            run(`build/main -m $map_file -i $scen_file -N $N -o $output_file -t $time_limit_sec -s $seed -l -v 1`)
            row = Dict(
                :solver => "Lacam",
                :num_agents => N,
                :map_name => last(split(map_file, "/")),
                :scen => scen,
                :scen_num => 1,
            )
            for line in readlines(output_file)
                m = match(r"soc=(\d+)", line)
                !isnothing(m) && (row[:soc] = parse(Int, m[1]))
                m = match(r"soc_lb=(\d+)", line)
                !isnothing(m) && (row[:soc_lb] = parse(Int, m[1]))
                m = match(r"makespan=(\d+)", line)
                !isnothing(m) && (row[:makespan] = parse(Int, m[1]))
                m = match(r"makespan_lb=(\d+)", line)
                !isnothing(m) && (row[:makespan_lb] = parse(Int, m[1]))
                m = match(r"comp_time=(\d+)", line)
                !isnothing(m) && (row[:comp_time] = parse(Int, m[1]))
                m = match(r"seed=(\d+)", line)
                !isnothing(m) && (row[:seed] = parse(Int, m[1]))
                m = match(r"solved=(\d+)", line)
                !isnothing(m) && (row[:solved] = parse(Int, m[1]))
            end
            push!(result, row)
            rm(output_file)
        end
    end
    result_file = joinpath(root_dir, "result.csv")
    CSV.write(result_file, result)
    nothing
end
