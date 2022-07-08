import CSV
import Dates
import Base.Threads
include("summary.jl")

function main()
    map_name_arr = ["random-32-32-20"]
    agents = [100, 200, 300, 400]
    scen_num = 10
    time_limit_sec = 30
    num_total_tasks = length(map_name_arr) * length(agents) * scen_num

    result = Vector{Any}(undef, num_total_tasks)
    cnt_fin = Threads.Atomic{Int}(0)
    loops = collect(enumerate(Iterators.product(map_name_arr, 1:scen_num, agents)))
    Threads.@threads for (k, (map_name_short, scen, N)) in loops
        output_file = "build/result.txt"
        map_name = joinpath(@__DIR__, "..", "assets", "map", "$(map_name_short).map")
        scen_name = joinpath(@__DIR__, "..", "assets", "scen", "local",
            "$(map_name_short)-random-$(scen).scen")
        run(`./build/main -m $map_name -i $scen_name -N $N -o $output_file -t $time_limit_sec`)
        Threads.atomic_add!(cnt_fin, 1)
        print("\r$(cnt_fin[])/$(num_total_tasks) tasks have been finished";)
        row = Dict(:N => N, :map_name => map_name_short, :scen => scen)
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
            m = match(r"solved=(\d+)", line)
            !isnothing(m) && (row[:solved] = parse(Int, m[1]))
        end
        result[k] = NamedTuple{Tuple(keys(row))}(values(row))
    end
    println()
    date_str = replace(string(Dates.now()), ":" => "-")
    root_dir = joinpath(@__DIR__, "..", "..", "data", "exp", date_str)
    !isdir(root_dir) && mkpath(root_dir)
    result_file = joinpath(root_dir, "result.csv")
    CSV.write(result_file, result)

    # display figures
    print_summary(result_file)
    plot_cactus(result_file)
end

main()
