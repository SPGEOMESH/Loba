



import numpy as np

from matplotlib import pyplot as plt
import matplotlib.gridspec as gridspec



COLOR_REF = "#2B2D42"
COLOR_MEAN = "#D90429"
COLOR_BOUNDS = "#8D99AE"
COLOR_STD = "#F77F00"



params = np.loadtxt("main_test_loadbalancer.params", skiprows=1)
nunits, nstations, mean, A = params[0], params[1], params[2], params[3]

plt.style.use('seaborn-v0_8-whitegrid' if 'seaborn-v0_8-whitegrid' in plt.style.available else 'default')
plt.rcParams['font.family'] = 'Times'
plt.rcParams['mathtext.fontset'] = 'stix'
fig = plt.figure(figsize=(10, 6), dpi=300)
gs = gridspec.GridSpec(nrows=2, ncols=1, figure=fig, hspace=0.3, wspace=0.25)

ax_ref = fig.add_subplot(gs[0, 0])
ax_reb = fig.add_subplot(gs[1, 0])


tag = 'ref'
ax = ax_ref
ymax = None

for it in range(2):

    X = np.loadtxt(f"main_test_loadbalancer.{tag}.dat")
    # data line: x, load at station 0, load at station 1, ... etc.
    xs = X[:,0]
    means = np.mean(X[:,1:], axis=1)
    maxs = np.max(X[:,1:], axis=1)
    mins = np.min(X[:,1:], axis=1)
    stds = np.std(X[:,1:], axis=1)
    work_vars = (A*(1-np.cos(xs))/2.0 + mean)*nunits/nstations

    # the same maximum for visual comparison
    if it == 0:
        ymax = maxs.max()

    # ax.fill_between(
    #     xs, mins, maxs,
    #     color=COLOR_BOUNDS, alpha=0.25,
    #     label="Model Range [$Y_{min}, Y_{max}$]"
    # )
    ax.fill_between(
        xs, means-stds, means+stds,
        color=COLOR_STD, alpha=0.5,
        label="$\pm \sigma$"
    )
    ax.plot(xs, maxs, color=COLOR_BOUNDS, linestyle="--", linewidth=1.2, alpha=0.8, label="$\ell_{max}$")
    ax.plot(xs, mins, color=COLOR_BOUNDS, linestyle=":", linewidth=1.2, alpha=0.8, label="$\ell_{min}$")
    ax.plot(xs, means, color=COLOR_MEAN, linestyle="-", linewidth=2.0, label="$\ell_{avg}$")
    # ax.plot(xs, work_vars, color=COLOR_REF, linestyle="-", linewidth=2.5, label="$\mu+\sigma$")

    ax.set_xlabel("$t$", fontsize=11, labelpad=8)
    ax.set_ylabel("load $\ell$", fontsize=11, labelpad=8)
    ax.grid(True, linestyle="--", alpha=0.5, color="#CCCCCC")

    ax.spines['top'].set_visible(False)
    ax.spines['right'].set_visible(False)
    ax.spines['left'].set_linewidth(0.8)
    ax.spines['bottom'].set_linewidth(0.8)

    ax.legend(
        loc="upper left",
        frameon=True,
        facecolor="white",
        edgecolor="#E0E0E0",
        framealpha=0.9,
        fontsize=9.5
    )
    ax.set_xlim((xs.min(), xs.max()))
    ax.set_ylim((0,ymax))

    tag = 'reb'
    ax = ax_reb


fig.suptitle(f'Load Balancer Test (n {nunits:.0f} N {nstations:.0f})', fontsize=16, fontweight='bold', color='#1A1A1A', y=0.98)


plt.tight_layout()

plt.savefig("main_test_loadbalancer.png", dpi=300, bbox_inches="tight")

# plt.show()
