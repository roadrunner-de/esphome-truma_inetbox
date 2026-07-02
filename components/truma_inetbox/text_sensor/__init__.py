from esphome.components import text_sensor
import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import CONF_ID, CONF_TYPE

from .. import truma_inetbox_ns, CONF_TRUMA_INETBOX_ID, TrumaINetBoxApp

DEPENDENCIES = ["truma_inetbox"]
CODEOWNERS = ["@Fabian-Schmidt"]

CONF_CLASS = "class"

TrumaTextSensor = truma_inetbox_ns.class_(
    "TrumaTextSensor", text_sensor.TextSensor, cg.Component
)

TRUMA_TEXT_SENSOR_TYPE_dummy_ns = truma_inetbox_ns.namespace("TRUMA_TEXT_SENSOR_TYPE")

CONF_SUPPORTED_TYPE = {
    "AIRCON_MODE": {
        CONF_CLASS: TRUMA_TEXT_SENSOR_TYPE_dummy_ns.AIRCON_MODE,
    },
    "AIRCON_FAN_MODE": {
        CONF_CLASS: TRUMA_TEXT_SENSOR_TYPE_dummy_ns.AIRCON_FAN_MODE,
    },
}

CONFIG_SCHEMA = text_sensor.text_sensor_schema().extend(
    {
        cv.GenerateID(): cv.declare_id(TrumaTextSensor),
        cv.GenerateID(CONF_TRUMA_INETBOX_ID): cv.use_id(TrumaINetBoxApp),
        cv.Required(CONF_TYPE): cv.enum(CONF_SUPPORTED_TYPE, upper=True),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await text_sensor.register_text_sensor(var, config)
    await cg.register_parented(var, config[CONF_TRUMA_INETBOX_ID])

    cg.add(var.set_type(CONF_SUPPORTED_TYPE[config[CONF_TYPE]][CONF_CLASS]))