# Currencies

![A GIF file showing a demonstration of plugin features. First, it shows that it allows conversion between currencies which the built-in Unit converter plugin doesn't support. Second, it shows that it works case-insensitive. Third, it shows that it doesn't display matches if currency conversion is supported by Unit Converter.](demo.gif)

Converts any currency to any other. Made because the built-in Unit Converter KRunner plugin uses very limited [data from ECB](https://www.ecb.europa.eu/stats/policy_and_exchange_rates/euro_reference_exchange_rates/html/index.en.html), only having about 30 currencies.

Doesn't show matches for ECB currencies, so you won't get duplicated results with both Unit Converter and this plugin enabled.

Exchange rates are updated either on KRunner restart/plugin reload, or daily.

[**Supported currencies list here.**](https://www.exchangerate-api.com/docs/supported-currencies)


### Installation
If you have KRunner installed, you already should have all dependencies.

```sh
git clone --depth=1 https://github.com/NotQuiteLoona/Currencies/
cd Currencies
sh install.sh
cd ..
rm -rf Currencies
```

### Uninstallation
```sh
git clone --depth=1 https://github.com/NotQuiteLoona/Currencies/
cd Currencies
sh uninstall.sh
cd ..
rm -rf Currencies
```

### Development
- Caching is completely implemented and supposed to work, but doesn't create files for some reason. I'm not a C++/Qt professional, so if anyone has answers why it doesn't work, I'll be happy to fix it.
- Settings menu is not present. However, settings themselves are implemented completely too.
- Besides that, there are only some little and non-visible code improvements to make before perfection - for example, DRY fixes, and probably some decluttering of the main class.

### Licensing
Licensed under the EUPL
