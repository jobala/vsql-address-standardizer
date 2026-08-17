# vsql-address-standardizer

Parses freeform US postal addresses into their components — house number, street, suffix, city, state & zip. This
extension was inspired by PostgreSQL's contrib [address_standardizer](https://github.com/postgis/address_standardizer)

## Installation

```sql
install extension vsql_address_standardizer
```

## Functions

### address_standardize

```sql
select address_standardize("123 north main burlington st south apt 4, springfield new york 62704-1234");


-- 123 N MAIN BURLINGTON ST S APT 4, SPRINGFIELD NY 62704-1234
```

### address_parse_json

```sql
 select address_parse_json("123 north main burlington st south apt 4 , springfield new york 62704-1234");

--  {"house_number":"123", "pre_directional":"N", "street_name":"MAIN BURLINGTON", "street_suffix":"ST", "post_directional":"S", "unit_designator":"APT", "unit_identifier":"4", "city":"SPRINGFIELD", "state":"NY", "zip":"62704-1234"}
```

### address_field

```sql
select address_field("123 north main burlington st south apt 4 , springfield new york 62704-1234", "city");

-- SPRINGFIELD
```

## License

[GPLv2](./LICENSE)
