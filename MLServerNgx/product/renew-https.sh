#!/bin/bash

certbot renew --force-renewal
service openresty reload
