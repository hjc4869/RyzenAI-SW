#pragma once

#include <ops/op_interface.hpp>
#include <ops/ops_common.hpp>

namespace ryzenai {

template <typename InT, typename WtT, typename OutT>
class mhachannel : public OpInterface {
private:
  std::map<std::string, std::string> txnbin_a_header;
  std::map<std::string, std::string> txnbin_b_header;
  std::map<std::string, std::string> txnbin_acc_header;
  std::map<std::string, std::vector<std::vector<size_t>>> default_shapes_;
  std::map<std::string, std::vector<std::vector<size_t>>> raw_shapes_;

  /* batch, head, x, y dimension of input to MHACHANNEL*/
  int64_t kernel_x_shape_[4];

  /* K x N dimension of base matmul being offloaded to AIE */
  int64_t kernel_y_shape_[2];
  /* M x N dimension of base matmul being offloaded to AIE */
  int64_t kernel_z_shape_[2];
  /*Kernel shape selected in runtime*/
  int64_t kernel_x_rows;
  /* Max Kernel M size supported for a given model*/
  int KERNEL_M_MAX;
  /* actual M x K of matrix A */
  int64_t a_shape_[2];
  /* actual M x N of matrix A */
  int64_t c_shape_[2];
  /* actual K x N of matrix A */
  int64_t w_shape_[2];
  /* padded shape of weight matrix */
  int64_t w_padded_shape_[2];
  /* xrt context handle */
  //   xrt_context *xrt_ctx_;
  //   static instruction_registry instr_reg_;
  static std::once_flag instr_reg_flag_;
  /* XRT BO for tiled activation matrix */
  xrt::bo a_bo_;
  /* XRT BO for tiled weight matrix */
  xrt::bo b_bo_;
  /* XRT BO for tiled output matrix */
  xrt::bo c_bo_;
  /* variables to store profile data */
  int64_t a_copy_time_;
  int64_t a_sync_time_;
  int64_t b_copy_time_;
  int64_t b_format_time_;
  int64_t b_sync_time_;
  int64_t c_copy_time_;
  int64_t c_sync_time_;
  int64_t run_aie_time_;
  int64_t cpu_acc_time_;
  int64_t num_run_aie_;
  uint64_t num_execute_ = 0;
  static std::once_flag logger_flag_;
  uint64_t mhachannel_id_;
  static uint64_t mhachannel_count;
  /* debug flag */
  bool debug_ = false;
  /* bo_type_ */
  int bo_type_ = 0;
  std::string a_dtype_;
  std::string b_dtype_;
  std::string c_dtype_;
  /* size for input activation dtype*/
  int a_dtype_size_;
  /* size for weights dtype*/
  int b_dtype_size_;
  /* size for output activation dtype*/
  int c_dtype_size_;
  std::string txn_fname_prefix_;
  std::string param_fname_prefix_;
  uint32_t const_pad_;

  /*
   * Utility function that setups the instruction registry with transaction
   * binaries.
   */
  void setup_instr_registry();

  std::string get_instr_key(std::string prefix, std::vector<size_t> &mat);
  std::tuple<size_t, size_t> map_padded_shape(int M, int N);

public:
  mhachannel(const std::string &a_dtype, const std::string &b_dtype,
             const std::string &c_dtype, bool load_xrt = true);
  void debug(bool enable);
  void set_params(const std::string &model_name,
                  std::vector<size_t> input_shape);
  const std::vector<uint8_t> get_transaction_bin(
      std::vector<Tensor> &input, std::vector<Tensor> &output,
      const std::map<std::string, std::any> &attr = {}) override;
  const std::vector<uint8_t> get_super_kernel_params(
      std::vector<Tensor> &input, std::vector<Tensor> &output,
      const std::map<std::string, std::any> &attr = {}) override;
  std::vector<OpArgMap>
  get_buffer_reqs(std::vector<Tensor> &input, std::vector<Tensor> &output,
                  const std::map<std::string, std::any> &attr = {}) override;
  void initialize_const_params(
      void *dst, const std::vector<Tensor> &const_params,
      const std::map<std::string, std::any> &attr = {}) override;
  void execute(const std::vector<Tensor> &input, std::vector<Tensor> &output);
  void
  initialize_const_params(const std::vector<Tensor> &const_params,
                          const std::map<std::string, std::any> &attr = {});
};

} // namespace ryzenai
