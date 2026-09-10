# sandbox

Two small projects: a convolutional network trained on FashionMNIST, and a
moving-average crossover backtest on real market data.

## ml_colab.ipynb

A CNN that classifies images of clothing into ten categories. Two
convolutional blocks followed by two linear layers, trained with Adam.

Written to run on Google Colab, where PyTorch and a GPU are available.
Trains on a 6,000 image subset by default so it finishes quickly on a CPU
runtime. Raise `TRAIN_SUBSET` to 60000 for the full dataset when a GPU is
attached.

Outputs are committed, so the results are visible without running anything.

## backtest.py

A 50/200 day moving-average crossover on SPY, from 2010 to 2024, using daily
prices from Yahoo Finance.

Signals are shifted by one day so that positions are only entered on the bar
after the signal is generated, which avoids look-ahead bias. Trading costs
are charged at 1 basis point on every position change. Reported metrics are
CAGR, annualised volatility, Sharpe ratio and maximum drawdown, alongside a
buy and hold benchmark.

The strategy underperforms buy and hold over this period. That is the
honest result and it is left as it is.

```
                  STRATEGY    BUY & HOLD
Total return        291.5%        545.9%
CAGR                10.10%        14.06%
Volatility          14.04%        16.93%
Sharpe ratio          0.76          0.86
Max drawdown        -33.7%        -33.7%
Trades made             13
```

Requires `yfinance`, `pandas`, `numpy` and `matplotlib`.

```
python backtest.py
```
