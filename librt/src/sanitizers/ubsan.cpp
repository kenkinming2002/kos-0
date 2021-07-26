#include <stdint.h>
#include <stddef.h>

#include <librt/Log.hpp>
#include <librt/Panic.hpp>

/* NOTE: Do not attempt to build with ubsan on release build unless you are okay
 *       with extremely long compile and link time */

namespace ubsan
{
  struct SourceLocation
  {
    const char *filename;
    uint32_t line;
    uint32_t column;

    void log() const
    {
      rt::logf("location: %s %lu:%lu\n", filename, (long unsigned)line, (long unsigned)column);
    }
  };


  struct TypeDescriptor
  {
    enum Kind : uint16_t
    {
      TK_Integer = 0x0000,
      TK_Float = 0x0001,
      TK_Unknown = 0xffff
    };

    Kind kind;
    uint16_t info;
    char typeName[];

    void log() const
    {
      rt::logf("type: %s %s\n", kind == TK_Integer ? "integer"
                              : kind == TK_Float ? "integer"
                              : "unknown", typeName);

    }
  };

  struct TypeMismatchData
  {
    enum class Kind : uint8_t
    {
    };

    SourceLocation location;
    const TypeDescriptor* type;
    uint8_t logAlignment;
    Kind kind;

    void log() const
    {
      location.log();
      type->log();
      rt::logf("alignment:%u\n", unsigned(logAlignment));
    }
  };

  extern "C"
  {
    void __ubsan_handle_add_overflow(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_add_overflow_abort(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_alignment_assumption(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_alignment_assumption_abort(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_builtin_unreachable(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_cfi_bad_type(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_cfi_check_fail(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_cfi_check_fail_abort(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_divrem_overflow(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_divrem_overflow_abort(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_dynamic_type_cache_miss(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_dynamic_type_cache_miss_abort(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_float_cast_overflow(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_float_cast_overflow_abort(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_function_type_mismatch_v1(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_function_type_mismatch_v1_abort(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_implicit_conversion(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_implicit_conversion_abort(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_invalid_builtin(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_invalid_builtin_abort(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_invalid_objc_cast(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_invalid_objc_cast_abort(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_load_invalid_value(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_load_invalid_value_abort(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_missing_return(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_mul_overflow(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_mul_overflow_abort(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_negate_overflow(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_negate_overflow_abort(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_nonnull_arg(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_nonnull_arg_abort(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_nonnull_return_v1(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_nonnull_return_v1_abort(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_nullability_arg(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_nullability_arg_abort(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_nullability_return_v1(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_nullability_return_v1_abort(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_out_of_bounds(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_out_of_bounds_abort(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_pointer_overflow(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_pointer_overflow_abort(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_shift_out_of_bounds(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_shift_out_of_bounds_abort(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_sub_overflow(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_sub_overflow_abort(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_type_mismatch_v1(TypeMismatchData* data, void* ptr)
    {
      if(reinterpret_cast<uintptr_t>(ptr) % (1 << data->logAlignment) != 0)
        return;

      data->log();
      rt::logf("pointer:0x%lx\n", reinterpret_cast<uintptr_t>(ptr));
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_type_mismatch_v1_abort(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_vla_bound_not_positive(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_handle_vla_bound_not_positive_abort(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_default_options(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_on_report(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }

    void __ubsan_get_current_report_data(...)
    {
      rt::panic("ubsan %s\n", __PRETTY_FUNCTION__);
    }
  }
}

