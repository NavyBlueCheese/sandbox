import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import yfinance as yf

TICKER = "SPY"
START = "2010-01-01"
END = "2024-12-31"
FAST = 50
SLOW = 200
COST_BPS = 1.0
TRADING_DAYS = 252


def load_prices(ticker, start, end):
    df = yf.download(ticker, start=start, end=end, progress=False, auto_adjust=True)
    if df.empty:
        raise SystemExit(f"No data came back for {ticker}. Check the ticker spelling.")
    if isinstance(df.columns, pd.MultiIndex):
        df.columns = df.columns.droplevel(1)
    return df[["Close"]].dropna()


def build_signal(df, fast, slow):
    df = df.copy()
    df["ma_fast"] = df["Close"].rolling(fast).mean()
    df["ma_slow"] = df["Close"].rolling(slow).mean()
    df["target"] = (df["ma_fast"] > df["ma_slow"]).astype(int)
    df["position"] = df["target"].shift(1).fillna(0)
    return df.dropna()


def run_backtest(df, cost_bps):
    df = df.copy()
    df["ret"] = df["Close"].pct_change().fillna(0.0)
    trades = df["position"].diff().abs().fillna(0.0)
    df["cost"] = trades * (cost_bps / 10_000.0)
    df["strat_ret"] = df["position"] * df["ret"] - df["cost"]
    df["equity_strategy"] = (1 + df["strat_ret"]).cumprod()
    df["equity_buyhold"] = (1 + df["ret"]).cumprod()
    return df


def performance(returns, equity):
    years = len(returns) / TRADING_DAYS
    total = equity.iloc[-1] - 1
    cagr = equity.iloc[-1] ** (1 / years) - 1
    vol = returns.std() * np.sqrt(TRADING_DAYS)
    sharpe = (returns.mean() * TRADING_DAYS) / vol if vol > 0 else np.nan
    drawdown = equity / equity.cummax() - 1
    max_dd = drawdown.min()
    return {
        "Total return": f"{total:>10.1%}",
        "CAGR": f"{cagr:>10.2%}",
        "Volatility": f"{vol:>10.2%}",
        "Sharpe ratio": f"{sharpe:>10.2f}",
        "Max drawdown": f"{max_dd:>10.1%}",
    }


def main():
    print(f"Downloading {TICKER} from {START} to {END} ...")
    prices = load_prices(TICKER, START, END)
    print(f"Got {len(prices)} daily bars.\n")

    df = build_signal(prices, FAST, SLOW)
    df = run_backtest(df, COST_BPS)

    strat = performance(df["strat_ret"], df["equity_strategy"])
    hold = performance(df["ret"], df["equity_buyhold"])
    n_trades = int(df["position"].diff().abs().sum())

    print(f"{TICKER}  |  {FAST}/{SLOW} moving-average crossover")
    print(f"{'':<14}{'STRATEGY':>12}{'BUY & HOLD':>14}")
    print("-" * 40)
    for k in strat:
        print(f"{k:<14}{strat[k]:>12}{hold[k]:>14}")
    print("-" * 40)
    print(f"{'Trades made':<14}{n_trades:>12}")

    _fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(11, 8), sharex=True,
                                   gridspec_kw={"height_ratios": [2, 1]})

    ax1.plot(df.index, df["equity_strategy"], label="Strategy", linewidth=1.6)
    ax1.plot(df.index, df["equity_buyhold"], label="Buy & hold", linewidth=1.2, alpha=0.75)
    ax1.set_ylabel("Growth of $1")
    ax1.set_title(f"{TICKER}: {FAST}/{SLOW} crossover vs buy & hold")
    ax1.legend()
    ax1.grid(alpha=0.3)

    dd = df["equity_strategy"] / df["equity_strategy"].cummax() - 1
    ax2.fill_between(df.index, dd, 0, alpha=0.4, color="crimson")
    ax2.set_ylabel("Drawdown")
    ax2.set_xlabel("Date")
    ax2.grid(alpha=0.3)

    plt.tight_layout()
    out = "backtest_result.png"
    plt.savefig(out, dpi=110)
    print(f"\nChart saved to {out}")
    plt.show()


if __name__ == "__main__":
    main()
